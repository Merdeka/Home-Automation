#include <EEPROM.h>

void setupEEPROM() {

  // Read interfaces struct from EEPROM address 512 and setup
  EEPROM.get(512, interfaces);

  // to disable interfaces from sketch with a #define
  #ifdef DISABLE_KNX
    interfaces.KNX    = false;
  #endif
  #ifdef DISABLE_RS485
    interfaces.RS485  = false;
  #endif
  #ifdef DISABLE_DSC
    interfaces.DSC    = false;
  #endif
  
  // Read debug struct from EEPROM address 1024 and setup
  EEPROM.get(1024, debug);

  // to enable debug from sketch with a #define
  #ifdef DEBUG
    debug.MAIN         = true;
  #endif
  #ifdef ETH_DEBUG
    debug.ETH          = true;
  #endif
  #ifdef MQTT_DEBUG
    debug.MQTT         = true;
  #endif
  #ifdef KNX_DEBUG
    debug.KNX          = true;
  #endif
  #ifdef DSC_DEBUG
    debug.DSC          = true;
  #endif
  #ifdef SOLADIN_DEBUG
    debug.SOLADIN;     = true;
  #endif
  #ifdef ONEWIRE_DEBUG
    debug.ONEWIRE      = true;
  #endif
  #ifdef POWER_DEBUG
    debug.POWER        = true;
  #endif
  #ifdef PLANTNODE_DEBUG
    debug.PLANTNODE    = true;
  #endif
  #ifdef LOCALSENSORS_DEBUG
    debug.LOCALSENSORS = true;
  #endif
}

void saveInterfaces() {
    // save interfaces struct from EEPROM address 512 and setup
  EEPROM.put(512, interfaces);
  if( debug.MAIN ) { Serial.println(F("Interface Struct Saved to EEPROM")); }
}

void saveDebug() {
    // save debug struct from EEPROM address 1024 and setup
  EEPROM.put(1024, debug);
  if( debug.MAIN ) { Serial.println(F("Debug Struct Saved to EEPROM")); }
}
