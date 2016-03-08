/*  
  # the sensor value description
  # 0  ~300     dry soil
  # 300~700     humid soil
  # 700~950     in water
*/

// Just Some basic Definitions used for the Up Time Logger
unsigned long    uptimeDay = 0;
uint8_t uptimeHour, uptimeMinute, uptimeSecond, uptimeSecondStamp, uptimeOnce = 0;

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "TypeDefsSariwating.h"

// Setup TypeDefs
PayloadPlantNode plantNode;

#include "jeenode.h"
#include "DHT.h"
#include "systemdiag.h"

#define DEBUG

unsigned long fast_update, middle_update, slow_update;

int SoilSensor  = A2;   // JeeNode Port 3 Analog
int COSensor    = A1;   // JeeNode Port 2 Analog
int COPower     = 5;    // JeeNode Port 2 Digital
#define DHTPIN  6       // JeeNode Port 3 Digital

#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

int co_preheat  = 0;
int co_pause    = 0;

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup() {

  pinMode(COPower, OUTPUT);

  // Serial Port
  #ifdef DEBUG
    Serial.begin(57600);
    Serial.print(F("Plant Node ID: "));
    Serial.println(MYNODE);
  #endif
    
  // JeeNode
  setupJeeNode();

  dht.begin();

  sensorupdate();
  
  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds
 }

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop() {

    wdt_reset();    // the program is alive...for now.
    uptime();       // Runs the uptime routine and reenters the main loop
    loopTimer();    // To Measure the loop time 

    //-------------------------------------------------------------------------------------------
    // Timed Code
    //-------------------------------------------------------------------------------------------
    // Every 10 Seconds
    if ((millis()-fast_update)>10000) { // 10 Seconds
    
      fast_update = millis();
    }
    
    // Every 30 Seconds
    if ((millis()-middle_update)>30000) { // 30 Seconds
        
      middle_update = millis();
      sensorupdate();
      doReport();
    }
    
    // Every 60 Seconds
    if ((millis()-slow_update)>60000) { // 60 Seconds
        
      slow_update = millis();
    }
}

//--------------------------------------------------------------------------------------------
// Update Sensors
//--------------------------------------------------------------------------------------------
static void sensorupdate() {
  
    plantNode.SoilSensor = analogRead(SoilSensor);
    Serial.print(F("Moisture Sensor:"));
    Serial.println(plantNode.SoilSensor);
  
    plantNode.Temperature = dht.readTemperature();
    plantNode.Humidity = dht.readHumidity();
  
    Serial.print(F("Humidity: "));
    Serial.print(plantNode.Humidity);
    Serial.print("%\t");
    Serial.print(F("Temperature: "));
    Serial.print(plantNode.Temperature);
    Serial.print(F(" *C "));
    Serial.println();

    plantNode.CO = analogRead(COSensor);
    Serial.print(F("Carbon Monoxide Sensor: "));
    Serial.println(plantNode.CO);
}

//--------------------------------------------------------------------------------------------
// periodic report, i.e. send out a packet and optionally report on serial port
//--------------------------------------------------------------------------------------------
static void doReport() {

    plantNode.uptimeDay    =  uptimeDay;
    plantNode.uptimeHour   =  uptimeHour;
    plantNode.uptimeMinute =  uptimeMinute;
    plantNode.uptimeSecond =  uptimeSecond;
    plantNode.freeRam      =  freeRam();
  
    rf12_sleep(RF12_WAKEUP);
    rf12_sendNow(0, &plantNode, sizeof plantNode);
    rf12_sendWait(1);
    rf12_sleep(RF12_SLEEP);
}
