#include <ICSC.h>

ICSC rs485(Serial3, 40, 2); // Serial port, NodeID, DE Pin

#define BUSRELAY 8

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void powerCallBack(unsigned char src, char command, unsigned char len, char *data) {

  payloadEmonTX power;
  power = *(payloadEmonTX*) data;

  DynamicJsonBuffer  powerBuffer;
  JsonObject& powerJson = powerBuffer.createObject();
  
  powerJson[F("Timestamp")]     = getTimeStamp(); 
  powerJson[F("GridImporting")] = power.importing;
  powerJson[F("GridPower")]     = power.power1;
  powerJson[F("GridVoltage")]   = (float) power.voltage / 100;

  if( debug.POWER ) { powerJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/gridpower", powerJson);

}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void soladinCallBack(unsigned char src, char command, unsigned char len, char *data) {

  payloadSoladin soladinInverter;
  soladinInverter = *(payloadSoladin*) data;

  DynamicJsonBuffer  soladinBuffer;
  JsonObject& soladinJson = soladinBuffer.createObject();

  soladinJson[F("Timestamp")]           = getTimeStamp(); 
  soladinJson[F("PVVoltage")]           = (float) soladinInverter.PVvolt / 10;
  soladinJson[F("PVCurrent")]           = (float) soladinInverter.PVamp  / 100;
  soladinJson[F("ACgridPower")]         = soladinInverter.ACgridPower;
  soladinJson[F("ACgridFreq")]          = (float) soladinInverter.ACgridFreq / 100;
  soladinJson[F("ACgridVolt")]          = soladinInverter.ACgridVolt;
  soladinJson[F("DeviceTemp")]          = soladinInverter.DeviceTemp;
  soladinJson[F("AlarmFlag")]           = soladinInverter.AlarmFlag;
  soladinJson[F("TotalPower")]          = (float) soladinInverter.TotalPower / 100;
  soladinJson[F("TotalOperationTime")]  = soladinInverter.TotalOperationTime;
  soladinJson[F("FW_ID")]               = soladinInverter.FWID;
  soladinJson[F("FW_version")]          = soladinInverter.FWversion;
  soladinJson[F("FW_date")]             = soladinInverter.FWdate;
  
  if( debug.SOLADIN ) { soladinJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/soladin", soladinJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void soladinNodeCallBack(unsigned char src, char command, unsigned char len, char *data) {

  payloadSoladinNode soladinNode;
  soladinNode = *(payloadSoladinNode*) data;
  
  DynamicJsonBuffer  soladinNodeBuffer;
  JsonObject& soladinNodeJson = soladinNodeBuffer.createObject();

  soladinNodeJson[F("Timestamp")]   = getTimeStamp(); 
  soladinNodeJson[F("Temperature")] = (float) soladinNode.Temp / 100;
  soladinNodeJson[F("Humidity")]    = (float) soladinNode.Humidity / 100;
  soladinNodeJson[F("FreeRam")]     = soladinNode.freeRam;
  soladinNodeJson[F("Looptime")]    = soladinNode.looptime;
  soladinNodeJson[F("Uptime")]      = makeUptime(soladinNode.uptimeDay, soladinNode.uptimeHour, soladinNode.uptimeMinute, soladinNode.uptimeSecond);
  
  if( debug.SOLADIN) { soladinNodeJson.prettyPrintTo(Serial); }
  
  sendMQTT("iot/sariwating/hardware/status/soladinnode", soladinNodeJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void plantNodeCallBack(unsigned char src, char command, unsigned char len, char *data) {

  PayloadPlantNode plantNode;
  plantNode = *(PayloadPlantNode*) data;
  
  DynamicJsonBuffer  plantNodeBuffer;
  JsonObject& plantNodeJson = plantNodeBuffer.createObject();

  plantNodeJson[F("Timestamp")]   = getTimeStamp(); 
  plantNodeJson[F("Temperature")] = plantNode.Temperature;
  plantNodeJson[F("Humidity")]    = plantNode.Humidity;
  plantNodeJson[F("SoilSensor")]  = plantNode.SoilSensor;
  plantNodeJson[F("CO")]          = plantNode.CO;
  
  if( debug.PLANTNODE ) { plantNodeJson.prettyPrintTo(Serial); }
  
  sendMQTT("iot/sariwating/jeenode/plantnode", plantNodeJson);

  DynamicJsonBuffer  plantNodeCpuBuffer;
  JsonObject& plantNodeCpuJson = plantNodeCpuBuffer.createObject();

  plantNodeCpuJson[F("Timestamp")]  = getTimeStamp(); 
  plantNodeCpuJson[F("FreeRam")]    = plantNode.freeRam;
  plantNodeCpuJson[F("Looptime")]   = plantNode.looptime;
  plantNodeCpuJson[F("Uptime")]     = makeUptime(plantNode.uptimeDay, plantNode.uptimeHour, plantNode.uptimeMinute, plantNode.uptimeSecond);

  if( debug.PLANTNODE ) { plantNodeCpuJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/hardware/status/plantnode", plantNodeCpuJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void oneWireCallBack(unsigned char src, char command, unsigned char len, char *data) {

  payloadOneWire oneWireData;
  oneWireData = *(payloadOneWire*) data;

  char* address = "00:00:00:00:00:00:00:00";
  sprintf(address,"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", oneWireData.address0, oneWireData.address1, oneWireData.address2, oneWireData.address3, oneWireData.address4, oneWireData.address5, oneWireData.address6, oneWireData.address7);
  
  DynamicJsonBuffer  onewireBuffer;
  JsonObject& onewireJson = onewireBuffer.createObject();

  onewireJson[F("Timestamp")]   = getTimeStamp(); 
  onewireJson[F("Address")]     = address;
  onewireJson[F("Temperature")] = (float) oneWireData.temperature / 100;

  if( debug.ONEWIRE ) { onewireJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/onewire", onewireJson);

  const char* Woonkamer       PROGMEM = "10:4a:b2:4b:01:08:00:24";
  const char* CV_Aanvoer      PROGMEM = "10:07:6d:d4:01:08:00:51";
  const char* CV_Retour       PROGMEM = "10:db:40:d4:01:08:00:fc";
  const char* Warmwater       PROGMEM = "10:e3:2f:4c:01:08:00:33";
  const char* OneWireJeeNode  PROGMEM = "10:5f:eb:ed:00:08:00:f9";
  
  if( (String) address == (String) Woonkamer ) { // Woonkamer Temp
    
    if( interfaces.KNX ) { Knx.write(3, (float) oneWireData.temperature / 100); }
  }
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void oneWireNodeCallBack(unsigned char src, char command, unsigned char len, char *data) {

    payloadOneWireNode oneWireNode;
    oneWireNode = *(payloadOneWireNode*) data;

    DynamicJsonBuffer  oneWireNodeBuffer;
    JsonObject& oneWireNodeJson = oneWireNodeBuffer.createObject();
    
    oneWireNodeJson[F("Timestamp")]   = getTimeStamp(); 
    oneWireNodeJson[F("Temperature")] = (float) oneWireNode.Temp / 100;
    oneWireNodeJson[F("FreeRam")]     = oneWireNode.freeRam;
    oneWireNodeJson[F("Looptime")]    = oneWireNode.looptime;
    oneWireNodeJson[F("Uptime")]      = makeUptime(oneWireNode.uptimeDay, oneWireNode.uptimeHour, oneWireNode.uptimeMinute, oneWireNode.uptimeSecond);
     
    if( debug.ONEWIRE ) { oneWireNodeJson.prettyPrintTo(Serial); }
  
    sendMQTT("iot/sariwating/hardware/status/onewirejeenode", oneWireNodeJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void shedIOCallBack(unsigned char src, char command, unsigned char len, char *data) {

    shedIO = *(payloadShed*) data;

    DynamicJsonBuffer  shedIOBuffer;
    JsonObject& shedIOJson = shedIOBuffer.createObject();
    
    shedIOJson[F("Timestamp")]   = getTimeStamp(); 
    shedIOJson[F("Temperature")] = (float) shedIO.Temp / 100;
    shedIOJson[F("Button1")]     = shedIO.button_1;
    shedIOJson[F("Button2")]     = shedIO.button_2;
    shedIOJson[F("Relay1")]      = shedIO.relay_1;
    shedIOJson[F("Relay2")]      = shedIO.relay_2;
     
    if( debug.SHEDNODE ) { shedIOJson.prettyPrintTo(Serial); }
  
    sendMQTT("iot/sariwating/shednode", shedIOJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void shedNodeCallBack(unsigned char src, char command, unsigned char len, char *data) {

    payloadShedNode shedNode;
    shedNode = *(payloadShedNode*) data;

    DynamicJsonBuffer  shedNodeBuffer;
    JsonObject& shedNodeJson = shedNodeBuffer.createObject();
    
    shedNodeJson[F("Timestamp")]   = getTimeStamp(); 
    shedNodeJson[F("FreeRam")]     = shedNode.freeRam;
    shedNodeJson[F("Looptime")]    = shedNode.looptime;
    shedNodeJson[F("Uptime")]      = makeUptime(shedNode.uptimeDay, shedNode.uptimeHour, shedNode.uptimeMinute, shedNode.uptimeSecond);
     
    if( debug.SHEDNODE ) { shedNodeJson.prettyPrintTo(Serial); }
  
    sendMQTT("iot/sariwating/hardware/status/shednode", shedNodeJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void setupRS485() {

  pinMode(BUSRELAY, OUTPUT);
  digitalWrite(BUSRELAY, interfaces.RS485BUS );
  
  Serial3.begin(57600); 
  
  rs485.begin();

  rs485.registerCommand(char(0x10), &powerCallBack);
  rs485.registerCommand(char(0x23), &soladinCallBack);
  rs485.registerCommand(char(0x24), &soladinNodeCallBack);
  rs485.registerCommand(char(0x25), &plantNodeCallBack);
  rs485.registerCommand(char(0x31), &oneWireCallBack);
  rs485.registerCommand(char(0x32), &oneWireNodeCallBack);
  rs485.registerCommand(char(0x45), &shedIOCallBack);
  rs485.registerCommand(char(0x46), &shedNodeCallBack);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void stopRS485() {
  
  Serial3.end();
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void enableRS485() {
  
  if( debug.MAIN ) { Serial.println(F("RS485 Protocol Enabled")); }
  
  interfaces.RS485 = true;
  setupRS485();
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void disableRS485() {
  
  if( debug.MAIN ) { Serial.println(F("RS485 Protocol Disabled")); }
  
  interfaces.RS485 = false;
  stopRS485();
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void sendRS485(char cmd, uint16_t len, char* packet) {

  rs485.broadcast(cmd, len, packet);
  
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void switchRS485Bus(bool value) {

  interfaces.RS485BUS = value;
  saveInterfaces();
  
  digitalWrite( BUSRELAY, interfaces.RS485BUS );
}

