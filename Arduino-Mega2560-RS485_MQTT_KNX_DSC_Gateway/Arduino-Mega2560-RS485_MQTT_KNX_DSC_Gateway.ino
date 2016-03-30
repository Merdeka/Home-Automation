#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "housekeeping.h"

unsigned long fast_update, middle_update, slow_update;

//--------------------------------------------------------------------------------------------
// Setup Routine
//--------------------------------------------------------------------------------------------
void setup() {

  debug.SHEDNODE = true;

  // Read EEPROM and setup Interfaces and Debug
  setupEEPROM();

  // Serial Port
  if( debug.MAIN ) {
    Serial.begin(57600);
    Serial.println();
    Serial.println(F("KNX-MQTT-RS485 Gateway"));
  }

                          setupEthernet();      // Setup Ethernet
                          setupMQTT();          // Setup MQTT
                          setupI2c();           // Setup i2c
  if( interfaces.KNX )    setupKNX();           // Setup KNX                              
  if( interfaces.RS485 )  setupRS485();         // Setup RS485
  if( interfaces.DSC )    setupDSC();           // Setup DSC                      
                          wdt_enable(WDTO_8S);  // Enable watchdog: max 8 seconds
}; // End Setup()

//--------------------------------------------------------------------------------------------
// Main Program Loop
//--------------------------------------------------------------------------------------------
void loop() {
    
                         wdt_reset();      // the program is alive...for now. 
                         uptime();         // Runs the uptime routine and reenters the main loop
                         loopTimer();      // To Measure the loop time
                         mqttTask();       // MQTT Loop
                         webServerTask();  // EtherNet WebServer Loop
  if( interfaces.KNX )   Knx.task();       // KNX Loop
  if( interfaces.RS485 ) rs485.process();  // RS485 Loop
  if( interfaces.DSC )   dscTask();        // DSC Alarm Loop
  

  //-------------------------------------------------------------------------------------------
  // Timed Code
  //-------------------------------------------------------------------------------------------
  // Every 10 Seconds
  if ((millis()-fast_update)>10000) { // 10 Seconds

    fast_update = millis();
    readSensors();   // BMP085/HTU21D-F/INA219B
    readPower();     // INA219
    dscPoll();
    if (mqtt.connected()) {
      sendStatus();
    }
  }

    // Every 30 Seconds
  if ((millis()-middle_update)>30000) { // 30 Seconds

    middle_update = millis();
  }

  // Every 60 minutes
  if ((millis()-slow_update)>36000000) { // 60 minutes
      
    slow_update = millis();
    getNTP();
    dscStatusRequest();
  }
 
}; // End Main Loop
