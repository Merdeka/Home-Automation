// Just Some basic Definitions used for the Up Time Logger
unsigned long    uptimeDay = 0;
uint8_t uptimeHour, uptimeMinute, uptimeSecond, uptimeSecondStamp, uptimeOnce = 0;

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "TypeDefsSariwating.h"

// Setup TypeDefs
payloadOneWire oneWire;
payloadOneWireNode oneWireNode;

// Predefine functions
void JeeRelay(unsigned char src, char command, unsigned char len, char *data);
uint16_t freeRam();

#include "rs485.h"
#include "jeenode.h"
#include "onewire.h"
#include "systemdiag.h"

//#define DEBUG
//#define ONEWIRE_DEBUG

unsigned long fast_update, middle_update, slow_update;

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup() {

  // Serial Port
  #ifdef DEBUG
    Serial.begin(57600);
    Serial.println(F("RS485/JeeNode/Onewire Base Node"));
  #endif

  // JeeNode
  setupJeeNode();
  
  // RS485
  setupRS485();

  // OneWire
  setupOnewire();

  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds

}

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop() {

    wdt_reset();      // the program is alive...for now. 
    uptime();         // Runs the uptime routine and reenters the main loop
    loopTimer();      // To Measure the loop time 
    jeeNodeTask();    // JeeNode Loop
    ICSC.process();   // RS485 Loop

    //-------------------------------------------------------------------------------------------
    // Timed Code
    //-------------------------------------------------------------------------------------------
    // Every 10 Seconds
    if ((millis()-fast_update)>10000) { // 10 Seconds
  
      fast_update = millis();
      onewireTask();
    }
  
    // Every 30 Seconds
    if ((millis()-middle_update)>30000) { // 30 Seconds
        
      middle_update = millis();
      sendSensorsReport();
    }
  
    // Every 60 Seconds
    if ((millis()-slow_update)>60000) { // 60 Seconds
        
      slow_update = millis();
      setupOnewire(); // Rescan OneWire Bus for new devices every 60 seconds
    }
  
}
