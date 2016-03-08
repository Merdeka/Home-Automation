#include <pgmspace.h>
#include <Wire.h>
#include "esp_housekeeping.h"
#include "mqtt.h"
#include "sensors.h"

//--------------------------------------------------------------------------------------------
// Vars
//--------------------------------------------------------------------------------------------
unsigned long fast_update, medium_update, slow_update;

//--------------------------------------------------------------------------------------------
// Setup Routine
//--------------------------------------------------------------------------------------------
void setup() {
  
  // Serial Port
  #ifdef DEBUG
    Serial.begin(57600);
    Serial.println();
    Serial.println(F("ESP-01 MQTT LightSensor"));
  #endif

  Wire.begin(0,2);        // SDA and SCL on ESP-01
  delay(10);

  setupRTC();             // Setup RTC
  setupSensors();         // Setup Sensors
  setupMQTT();            // Setup MQTT
  storeChipId();          // Setup MQTT

  wdt_enable(WDTO_8S);    // Enable watchdog: max 8 seconds
}// End Setup()

//--------------------------------------------------------------------------------------------
// Main Program Loop
//--------------------------------------------------------------------------------------------
void loop() {

    
  wdt_reset();  // the program is alive...for now.
  uptime();     // Runs the uptime routine and reenters the main loop
  wifiTask();   // Wifi Loop
  mqttTask();   // MQTT Loop

  //-------------------------------------------------------------------------------------------
  // Timed Code
  //-------------------------------------------------------------------------------------------
  // Every 10 Seconds
  if ((millis()-fast_update)>10000) { // 10 Seconds

    fast_update = millis();
    getTSL2561();
    sensorData.temperature = htu.readTemperature() * 100;
    sensorData.humidity    = htu.readHumidity()    * 100;
    sendSensors();
  }

  // Every 30 Seconds
  if ((millis()-medium_update)>30000) { // 30 Seconds
      
    medium_update = millis();
    sendStatus();
  }

  // Every 60 minutes
  if ((millis()-slow_update)>36000000) { // 60 minutes
      
    slow_update = millis();
    getNTP();
  }

}// End Main Loop
