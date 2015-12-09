// RS485 Soladin Node, talks to Soladin 600 Inverter

#include <JeeLib.h>
#include <avr/wdt.h>

#include <Soladin.h>
#include <SoftwareSerial.h>
#include <ICSCSoftware.h>
#include "DHT.h"

const unsigned char NodeID = 33;
const int DEPin = 16;
const long Baud = 57600;

SoftwareSerial RS485(6, 5); // RX, TX

#define DHTPIN 7     // what pin we're connected to
DHT dht(DHTPIN, DHT11);

#define MEASURE_PERIOD  15 // how often to measure, in tenths of seconds
#define RETRY_PERIOD    10  // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5   // maximum number of times to retry
#define ACK_TIME        10  // number of milliseconds to wait for an ack
#define REPORT_EVERY    2   // report every N measurement cycles

// The scheduler makes it easy to perform various tasks at various times:

enum { MEASURE, REPORT, TASK_END };

static word schedbuf[TASK_END];
Scheduler scheduler (schedbuf, TASK_END);

// Other variables used in various places in the code:
int temp, humidity;

static byte reportCount;    // count up until next report, i.e. packet send

// This defines the structure of the packets which get sent out by RS485:

typedef struct {
    int16_t PVvolt;
    int16_t PVamp;
    int16_t ACgridPower;
    int16_t ACgridFreq;
    int16_t ACgridVolt;
    int16_t DeviceTemp;
    int16_t AlarmFlag;
    int16_t TotalPower;
    int16_t TotalOperationTime;
    int16_t DHT11Temp;
    int16_t DHT11Humidity;    
} PayloadSoladin;
PayloadSoladin soladin;

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

unsigned long slow_update;

SoftwareSerial solcom(14, 4);                            // serial to conect to soladin1 (AIO1 TX, DIO1 RX)

Soladin sol;						  // copy of soladin class
boolean connectSol = 0 ;				  // if soladin responds

static void doMeasure(){
  
  if (!connectSol) {      			        // Try to connect
    
    solcom.begin(9600);
    sol.begin(&solcom);
    
    Serial.println("Soladin Probe.....");
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(PRB)) {				// Try connecting to slave
        connectSol = true;
        Serial.println("Soladin connected");
        delay(50);
        sol.PVvolt = 0;
        sol.PVamp = 0;
        sol.Gridpower = 0;
        sol.Gridfreq = 0;
        sol.Gridvolt = 0;
        sol.DeviceTemp = 0;
        sol.Flag = 0;
        sol.Totalpower = 0;
        sol.TotalOperaTime = 0;
  
        doDS();
 
        soladin.PVvolt = sol.PVvolt;
        soladin.PVamp = sol.PVamp;
        soladin.ACgridPower = sol.Gridpower;
        soladin.ACgridFreq = sol.Gridfreq;
        soladin.ACgridVolt = sol.Gridvolt;
        soladin.DeviceTemp = sol.DeviceTemp;
        soladin.AlarmFlag = sol.Flag;
        soladin.TotalPower = sol.Totalpower;
        soladin.TotalOperationTime = sol.TotalOperaTime;     
  
        Serial.print("Soladin: ");
        Serial.print(soladin.PVvolt);
        Serial.print(" Volt, ");
        Serial.print(soladin.PVamp);
        Serial.print( " Amp, ");
        Serial.print(soladin.ACgridPower);
        Serial.println(" Watt");
        
        soladin.DHT11Temp = temp;
        soladin.DHT11Humidity = humidity;
   
        Serial.print("DHT11: ");
        Serial.print(temp);
        Serial.print(" C, ");
        Serial.print(humidity);
        Serial.println( " %, ");
        
        delay(250);
        solcom.end();
        connectSol = false;
        break;
      }
//      delay(1000);
    }
  }
}

static void serialFlush() {
    #if ARDUINO >= 100
        Serial.flush();
    #endif  
    delay(2); // make sure tx buf is empty before going back to sleep
}

// periodic report, i.e. send out a packet and optionally report on serial port
static void doReport() {
  
    char buffer[sizeof soladin];
    memcpy(buffer, &soladin, sizeof soladin);   
    ICSC.send(1, char(0x23), sizeof(soladin), buffer);
   
    Serial.println("Sending Report");
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

void setup()
{
    Serial.begin(57600);
    Serial.println("RS485 Soladin Node.");
    
    ICSC.begin(NodeID, Baud, &RS485, DEPin);
    dht.begin();
    
    serialFlush();
    
    reportCount = REPORT_EVERY;     // report right away for easy debugging
    scheduler.timer(MEASURE, 0);    // start the measurement loop going
    
    wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds
}

void loop()
{
      ICSC.process();
      switch (scheduler.pollWaiting()) {

        case MEASURE:
            // reschedule these measurements periodically
            scheduler.timer(MEASURE, MEASURE_PERIOD);
    
            doMeasure();

            // every so often, a report needs to be sent out
            if (++reportCount >= REPORT_EVERY) {
                reportCount = 0;
                scheduler.timer(REPORT, 0);
            }
            break;
            
        case REPORT:
            doReport();
            break;
    }
   
    if ((millis()-slow_update)>5000) {
      
     slow_update = millis();
     
     int t = dht.readTemperature();
     int h = dht.readHumidity();
     
     if (isnan(t) || isnan(h)) {
     }
     else {
       temp = t;
       humidity = h;
     }
    }
  wdt_reset();           // Reset watchdog    
}


static void doFW(){
  if (connectSol)  {      				// already connected
    for (int i=0 ; i < 4 ; i++) {			// give me some time to return by hand
      if (sol.query(FWI)) {				// request firware information
        break; 
      } 
    }
  }
}

static void doDS(){
  if (connectSol) {
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(DVS)) {				// request Device status       
        break;
      }
    } 
  }
}

