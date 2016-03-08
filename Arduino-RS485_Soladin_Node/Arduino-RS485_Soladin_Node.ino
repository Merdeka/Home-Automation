// RS485 Soladin Node, talks to Soladin 600 Inverter

// Just Some basic Definitions used for the Up Time Logger
unsigned long    uptimeDay = 0;
uint8_t uptimeHour, uptimeMinute, uptimeSecond, uptimeSecondStamp, uptimeOnce = 0;

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "TypeDefsSariwating.h"

// Setup TypeDefs
payloadSoladin soladin;
payloadSoladinNode soladinNode;

// Predefine functions
uint16_t freeRam();
String getUptime();

#include "rs485.h"
#include "soladin.h"
#include "sensors.h"
#include "systemdiag.h"

#define DEBUG
//#define SERIAL

unsigned long fast_update, slow_update;

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup() {

  // Serial Port
  #ifdef DEBUG
      Serial.begin(57600);
    Serial.println(F("RS485 Soladin Node."));
  #endif
  
  setupRS485();   // RS485
  setupSensors(); // DHT11

  doSoladinMeasure();
  sendSoladinReport();

  localSensors();  // DHT11
  sendNodeReport();

  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds

}

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop() {

    wdt_reset();      // the program is alive...for now. 
    uptime();         // Runs the uptime routine and reenters the main loop
    loopTimer();      // To Measure the loop time 
    ICSC.process();   // RS485 Loop

  //-------------------------------------------------------------------------------------------
  // Timed Code
  //-------------------------------------------------------------------------------------------
  // Every 10 Seconds
  if ((millis()-fast_update)>10000) { // 10 Seconds

    fast_update = millis();
    doSoladinMeasure();
    sendSoladinReport();
  }

  // Every 30 Seconds
  if ((millis()-slow_update)>30000) { // 30 Seconds
      
    slow_update = millis();
    localSensors();  // DHT11
    sendNodeReport();
  }
  
}
