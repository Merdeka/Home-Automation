/*  
  # the sensor value description
  # 0  ~300     dry soil
  # 300~700     humid soil
  # 700~950     in water
*/

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <JeeLib.h>
#include "DHT.h"
#include "TypeDefsSariwating.h"

// Just Some basic Definitions used for the Up Time Logger
long    uptimeDay = 0;
uint8_t uptimeHour, uptimeMinute, uptimeSecond, uptimeSecondStamp, uptimeOnce = 0;

// TypeDefs
PayloadPlantNode plantNode;

int SoilSensor = A2;  // JeeNode Port 3 Analog
int COSensor = A1;    // JeeNode Port 2 Analog
int COPower = 5;      // JeeNode Port 2 Digital
#define DHTPIN 6      // JeeNode Port 3 Digital

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 25 // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ // frequency - match to same frequency as RFM12B module
#define group 30 // network group, must be same as emonTx and emonBase


#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

int co_preheat = 0;
int co_pause = 0;

unsigned long loop_delay, sensor_update;

void setup() {

  pinMode(COPower, OUTPUT);

  rf12_initialize(MYNODE, freq, group);
  
  Serial.begin(57600);

  Serial.print(F("Plant Node ID: "));
  Serial.println(MYNODE);
  dht.begin();

  sensorupdate();
  
  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds
 }

void loop() {

  wdt_reset();    // the program is alive...for now.
  uptime();       // Runs the uptime routine and reenters the main loop

  //-------------------------------------------------------------------------------------------
  // Update every 1000ms
  //-------------------------------------------------------------------------------------------
  
  if ((millis()-loop_delay)>1000) {

      loop_delay = millis();

/*
      if(co_pause <= 10) {
        Serial.print("CO sensor pause: ");
        Serial.print(co_pause);
        Serial.println(" Sec");    
        co_pause++;
      } else {
         if(co_preheat <= 60) {
          Serial.print("CO sensor heating: ");
          Serial.print(co_preheat);
          Serial.println(" Sec");
          digitalWrite(COPower, HIGH);
          co_preheat++;
        } else {
          plantNode.CO = analogRead(COSensor);
          Serial.print("Carbon Monoxide Sensor:");
          Serial.println(plantNode.CO);
          digitalWrite(COPower, LOW);
          co_preheat = 0;
          co_pause = 0;
        }
*/        

     if ((millis()-sensor_update)>30000) { // 30 Seconds
      
      sensor_update = millis();

      sensorupdate();
      doReport();
     
     }
      
    }
}

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

// periodic report, i.e. send out a packet and optionally report on serial port
static void doReport() {

    plantNode.uptimeDay    = uptimeDay;
    plantNode.uptimeHour   = uptimeHour;
    plantNode.uptimeMinute = uptimeMinute;
    plantNode.uptimeSecond = uptimeSecond;
    plantNode.freeRam      = freeRam();
  
    rf12_sleep(RF12_WAKEUP);
    rf12_sendNow(0, &plantNode, sizeof plantNode);
    rf12_sendWait(1);
    rf12_sleep(RF12_SLEEP);
}
