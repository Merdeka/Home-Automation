#define DEBUG
//#define ETH_DEBUG
//#define MQTT_DEBUG
//#define KNX_DEBUG
//#define DSC_DEBUG
//#define SOLADIN_DEBUG
//#define ONEWIRE_DEBUG
//#define POWER_DEBUG
//#define PLANTNODE_DEBUG
//#define LOCALSENSORS_DEBUG
#define DSC_TROUBLE

//#define DISABLE_KNX
//#define DISABLE_RS485
//#define DISABLE_DSC

#include "TypeDefsSariwating.h"
#include <ArduinoJson.h>

// Setup TypeDefs
typedef struct {  // debug | also used to read/store EEPROM
  bool  MAIN;
  bool  ETH;
  bool  MQTT;
  bool  KNX;
  bool  RS485;
  bool  DSC;
  bool  SOLADIN;
  bool  ONEWIRE;
  bool  POWER;
  bool  PLANTNODE;
  bool  LOCALSENSORS;
  bool  SHEDNODE;  
}Debugs;
Debugs debug;

typedef struct {  // Interfaces | also used to read/store EEPROM
  bool  KNX;
  bool  RS485;
  bool  RS485BUS;
  bool  DSC;
}Interfaces;
Interfaces interfaces;

typedef struct {  // Local Sensors
  uint16_t      temperature;
  uint16_t      humidity;
  float         barometer;
  float         voltage;
  float         current;
  unsigned long looptime; 
}localSensors;
localSensors sensors;

payloadShed shedIO;

// Functions prototypes
void      sendKNXTime();
void      sendKNXDate();
void      sendMQTT(const char* topic, JsonObject& sendJson);
void      enableKNX();
void      disableKNX();
void      enableRS485();
void      switchRS485Bus(bool value);
void      disableRS485();
void      enableDSC();
void      disableDSC();
void      dscStatusRequest();
uint16_t  freeRam();

#include "eeprom_memory.h"
#include "eth.h"
#include "time.h"
#include "sensors.h"
#include "systemdiag.h"
#include "knx.h"
#include "rs485.h"
#include "mqtt.h"

