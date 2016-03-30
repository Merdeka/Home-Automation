#include <pgmspace.h>
#include "esp_housekeeping.h"
#include "display.h"
#include "mqtt.h"
#include "buttons.h"

//--------------------------------------------------------------------------------------------
// Vars
//--------------------------------------------------------------------------------------------
unsigned long fast_update, medium_update, slow_update;

//--------------------------------------------------------------------------------------------
// Main Setup
//--------------------------------------------------------------------------------------------
void setup() {
  
    Serial.begin(57600);
    Serial.println(F("MQTT OLED"));
    
    // OLED
    display.begin(true);
    display.setTextColor(WHITE);

    LEDS.begin();
    LEDS.setBrightness(32);

    setupButtons();         // Buttons
    setupMQTT();            // Setup MQTT
    storeChipId();
    drawOLED();

    wdt_enable(WDTO_8S);    // Enable watchdog: max 8 seconds

}

//--------------------------------------------------------------------------------------------
// Main Program Loop
//--------------------------------------------------------------------------------------------
void loop() {

    wdt_reset();  // the program is alive...for now.
    uptime();     // Runs the uptime routine and reenters the main loop
    loopTimer();  // To Measure the loop time
    wifiTask();   // Wifi Loop
    mqttTask();   // MQTT Loop

    if ( !mqtt.connected() || SETTINGS || (millis()-mqttConnected)<3000 ) {
      currentMode = settingsMode;
    } else {
      currentMode = storedMode;
    }

    drawOLED();
    buttonTask(); // Button Loop
    ledTask();
  
    //-------------------------------------------------------------------------------------------
    // Timed Code
    //-------------------------------------------------------------------------------------------    
    // Every 10 Seconds
    if ((millis()-fast_update)>10000) { // 10 Seconds
  
      fast_update = millis();
      sendStatus();
    }
  
    // Every 30 Seconds
    if ((millis()-medium_update)>30000) { // 30 Seconds
        
      medium_update = millis();
    }
  
    // Every 60 minutes
    if ((millis()-slow_update)>36000000) { // 60 minutes
        
      slow_update = millis();
      getNTP();
    }

}// End Main Loop
