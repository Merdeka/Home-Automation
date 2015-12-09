// Soladin Node, talks to Soladin 600 Inverter

#include <JeeLib.h>
#include <avr/sleep.h>

#include <PortsLCD.h>

#include <Soladin.h>
#include <SoftwareSerial.h>

#define SERIAL  1   // set to 1 to also report readings on the serial port

#define MEASURE_PERIOD  10 // how often to measure, in tenths of seconds
#define RETRY_PERIOD    10  // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5   // maximum number of times to retry
#define ACK_TIME        10  // number of milliseconds to wait for an ack
#define REPORT_EVERY    5   // report every N measurement cycles

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

// The scheduler makes it easy to perform various tasks at various times:

enum { MEASURE, REPORT, TASK_END };

static word schedbuf[TASK_END];
Scheduler scheduler (schedbuf, TASK_END);

// Other variables used in various places in the code:

static byte reportCount;    // count up until next report, i.e. packet send

#define MYNODE 11            //Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     //frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 30             //network group, must be same as emonTx and emonBase

// This defines the structure of the packets which get sent out by wireless:

typedef struct {
    byte NodeType;  // NodeType &H?? = Soladin Node
    int PVvolt;
    int PVamp;
    int ACgridPower;
    int ACgridFreq;
    int ACgridVolt;
    int DeviceTemp;
    int AlarmFlag;
    int TotalPower;
    int TotalOperationTime;
    byte lobat :1;  // supply voltage dropped under 3.1V: 0..1
} PayloadSoladin;
PayloadSoladin soladin;

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

PortI2C myI2C (1);                                      // LCD Port 1
LiquidCrystalI2C lcd (myI2C);

SoftwareSerial solcom(5, 6);                            // serial to conect to soladin (DIO3 RX, DIO2 TX) 
Soladin sol;						// copy of soladin class
boolean connect = 0 ;					// if soladin respons

static void doMeasure(){
  
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
 
  soladin.NodeType = 0x16;
  soladin.PVvolt = sol.PVvolt;
  soladin.PVamp = sol.PVamp;
  soladin.ACgridPower = sol.Gridpower;
  soladin.ACgridFreq = sol.Gridfreq;
  soladin.ACgridVolt = sol.Gridvolt;
  soladin.DeviceTemp = sol.DeviceTemp;
  soladin.AlarmFlag = sol.Flag;
  soladin.TotalPower = sol.Totalpower;
  soladin.TotalOperationTime = sol.TotalOperaTime;
  
  soladin.lobat = rf12_lowbat();
 
  lcd.setCursor(0, 1);
  lcd.print("PV: ");
  if((float(sol.PVvolt)/10) < 100){
    lcd.print("0");
  }
    if((float(sol.PVvolt)/10) < 10){
    lcd.print("0");
  }
  lcd.print(float(sol.PVvolt)/10);
  lcd.print("V ");
  lcd.print(float(sol.PVamp)/100);
  lcd.print("A");
   
  lcd.setCursor(0, 2);
  lcd.print("AC: ");
  lcd.print(sol.Gridvolt);
  lcd.print("V ");
  lcd.print(float(sol.Gridfreq)/100);
  lcd.print("Hz");
  
  lcd.setCursor(0, 3);
  lcd.print("Power: ");
  if(sol.Gridpower < 100){
    lcd.print("0");
  }
    if(sol.Gridpower < 10){
    lcd.print("0");
  }
  lcd.print(sol.Gridpower);
  lcd.print(" Watt ");
  lcd.setCursor(17, 0);
  if(sol.DeviceTemp < 10){
    lcd.print("0");
  }
  lcd.print(sol.DeviceTemp);
  lcd.print("C");
}

static void serialFlush() {
    #if ARDUINO >= 100
        Serial.flush();
    #endif  
    delay(2); // make sure tx buf is empty before going back to sleep
}

// periodic report, i.e. send out a packet and optionally report on serial port
static void doReport() {
    rf12_sleep(RF12_WAKEUP);
    while (!rf12_canSend())
        rf12_recvDone();
    rf12_sendStart(0, &soladin, sizeof soladin, RADIO_SYNC_MODE);
    rf12_sleep(RF12_SLEEP);
   
   
    Serial.println("Sending Report");
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

void setup()
{
    rf12_initialize(MYNODE, freq,group);
      
    Serial.begin(57600);
    Serial.print("[Soladin Node #");
    Serial.print(MYNODE);
    Serial.println("]");
    
    serialFlush();
      
    solcom.begin(9600);
    sol.begin(&solcom);
    
    lcd.backlight();
    // set up the LCD's number of rows and columns: 
    lcd.begin(20, 4);
    // Print a message to the LCD.
    lcd.setCursor(0, 0);
    lcd.print("Soladin Node #");
    lcd.print(MYNODE);
    
    rf12_sleep(RF12_SLEEP); // power down
    
    reportCount = REPORT_EVERY;     // report right away for easy debugging
    scheduler.timer(MEASURE, 0);    // start the measurement loop going
}

void loop()
{
   if (!connect) {      			        // Try to connect
    Serial.print("Soladin 600");
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(PRB)) {				// Try connecting to slave
        connect = true; 
        Serial.println("...Connected");
          doFW();
          delay(1000);
          SPrintFW();
        break;
      }
      Serial.print(".");
      delay(1000);
    }
  }
  
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
}


void doFW(){
  if (connect)  {      					// already connected
    for (int i=0 ; i < 4 ; i++) {			// give me some time to return by hand
      if (sol.query(FWI)) {				// request firware information
        break; 
      } 
    }
  }
}

void doDS(){
  if (connect) {
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(DVS)) {				// request Device status       
        break;
      }
    } 
  }
}

void SPrintFW() {
  Serial.print("FW ID= ");
  Serial.println(byte(sol.FW_ID),HEX);
  Serial.print("Ver= ");
  Serial.println(word(sol.FW_version),HEX);
  Serial.print("Date= ");
  Serial.println(word(sol.FW_date),HEX);
  Serial.println();
}

void SPrintDS() {
  Serial.print("PV= ");
  Serial.print(float(sol.PVvolt)/10);
  Serial.print("V;   ");
  Serial.print(float(sol.PVamp)/100);
  Serial.println("A");

  Serial.print("AC= ");
  Serial.print(sol.Gridpower);
  Serial.print("W;  ");
  Serial.print(float(sol.Gridfreq)/100);
  Serial.print("Hz;  ");
  Serial.print(sol.Gridvolt);
  Serial.println("Volt");
  
  Serial.print("Device Temperature= ");
  Serial.print(sol.DeviceTemp);
  Serial.println(" Celcius");
  
  Serial.print("AlarmFlag= ");
  Serial.println(sol.Flag,BIN);
  
  Serial.print("Total Power= ");
  Serial.print(float(sol.Totalpower)/100);
  Serial.println("kWh");
  // I really don't know, wy i must split the sprintf ?
  Serial.print("Total Operating time= ");
  char timeStr[14];
  sprintf(timeStr, "%04d:",(sol.TotalOperaTime/60));
  Serial.print(timeStr); 
  sprintf(timeStr, "%02d hh:mm ",  (sol.TotalOperaTime%60));
  Serial.println(timeStr);
  Serial.println();
  
  if (sol.Flag != 0x00) {				// Print error flags
    SPrintflag(); 
  }
 }

void SPrintflag(){
  if ( sol.Flag & 0x0001 ){
    Serial.println("Usolar too high"); 
  }
  if( sol.Flag & 0x0002 ){
    Serial.println("Usolar too low"); 
  }
  if( sol.Flag & 0x0004 ){
    Serial.println("No Grid"); 
  }
  if( sol.Flag & 0x0008 ){
    Serial.println("Uac too high"); 
  }   
  if( sol.Flag & 0x0010 ){
    Serial.println("Uac too low"); 
  }      
  if( sol.Flag & 0x0020 ){
    Serial.println("Fac too high"); 
  }      
  if( sol.Flag & 0x0040 ){
    Serial.println("Fac too low"); 
  }  
  if( sol.Flag & 0x0080 ){
    Serial.println("Temperature to high"); 
  }  
  if( sol.Flag & 0x0100 ){
    Serial.println("Hardware failure"); 
  } 
  if( sol.Flag & 0x0200 ){
    Serial.println("Starting"); 
  }   
  if( sol.Flag & 0x0400 ){
    Serial.println("Max Power"); 
  }   
  if( sol.Flag & 0x0800 ){
    Serial.println("Max current"); 
  }      
}
