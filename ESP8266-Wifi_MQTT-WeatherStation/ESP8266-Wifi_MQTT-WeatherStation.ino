#include <pgmspace.h>
#include "esp_housekeeping.h"
#include "mqtt.h"
#include "sensors.h"

//--------------------------------------------------------------------------------------------
// Vars
//--------------------------------------------------------------------------------------------
unsigned long fast_update, medium_update, slow_update, ws_update, ow_update;

//--------------------------------------------------------------------------------------------
// Setup Routine
//--------------------------------------------------------------------------------------------
void setup() {
  
  // Serial Port
  #ifdef DEBUG
    Serial.begin(57600);
    Serial.println();
    Serial.println(F("ESP-01 MQTT Weather Station"));
  #endif

  // Load Config from SPIFFS
  Serial.println(F("Mounting FS..."));

  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount file system"));
    return;
  }
  
  if (!loadConfig()) {
    Serial.println(F("Failed to load config"));
  } else {
    Serial.println(F("Config loaded"));
  }

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
  loopTimer();  // To Measure the loop time
  wifiTask();   // Wifi Loop
  mqttTask();   // MQTT Loop

  //-------------------------------------------------------------------------------------------
  // Timed Code
  //-------------------------------------------------------------------------------------------
  // Every 250 Milliseconds
  if ((millis()-ws_update)>250) { // 250 Milliseconds

    ws_update = millis();
      broadcastWS();
  }

  // Every 2.5 Seconds
  if ((millis()-ow_update)>2500) { // 2.5 Seconds

    ow_update = millis();   
      sensorTask(); // Sensor Loop
  }
  
  // Every 10 Seconds
  if ((millis()-fast_update)>10000) { // 10 Seconds

    fast_update = millis();
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
