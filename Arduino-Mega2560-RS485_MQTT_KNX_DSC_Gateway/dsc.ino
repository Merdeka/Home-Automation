#include "dsccommands.h"

String rxBuffer;

//--------------------------------------------------------------------------------------------
// DSC Setup Routine
//--------------------------------------------------------------------------------------------
void setupDSC() {

  Serial2.begin(9600);

  if (Serial2) {
    dscPoll();
    Serial.flush();
    enableDescriptiveArming();
    dscStatusRequest();
  }
}

//--------------------------------------------------------------------------------------------
// DSC Stop Routine
//--------------------------------------------------------------------------------------------
void stopDSC() {
  
  Serial2.end();
}

//--------------------------------------------------------------------------------------------
// DSC Enable Routine
//--------------------------------------------------------------------------------------------
void enableDSC() {
  if( debug.MAIN ) { Serial.println(F("DSC Protocol Enabled")); }
  interfaces.DSC = true;
  setupDSC();
}

//--------------------------------------------------------------------------------------------
// DSC Disbale Routine
//--------------------------------------------------------------------------------------------
void disableDSC() {
    if( debug.MAIN ) { Serial.println(F("DSC Protocol Disabled")); }
    interfaces.DSC = false;
    stopDSC();
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void sendDSCtoKNX(uint8_t zone, uint8_t cmd) {
    if(zone == 2 || zone == 3) {
      Knx.write(5, cmd);
    }
}

//--------------------------------------------------------------------------------------------
// Main DSC Loop
//--------------------------------------------------------------------------------------------
void dscTask() {

  if ( Serial2.available() >0 ) {

    char data = Serial2.read();

      // Process message when line feed character is recieved
      if (data == (char) 0x0D )
      {
        rxBuffer.trim();           
        parseBuffer(rxBuffer);
  
        rxBuffer = ""; // clear RX Buffer
       } else {
        rxBuffer += data;
       }
  }
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void parseBuffer(String input) {

  uint8_t len = input.length();
  String command = input.substring(0, 3); 
  String checksum = input.substring(len-2);
  String data = input.substring(3, len-2);

  if( debug.DSC ) {
    Serial.print(F("Input: "));
    Serial.print(input);
    Serial.print(F(" Len: "));
    Serial.print(len);
    Serial.print(F(" Cmd: "));
    Serial.print(command);
    Serial.print(F(" CS: "));
    Serial.print(checksum);
    Serial.print(F(" Data: "));
    Serial.println(data);
  }
  
  if ( DSC_CS_OK(command, data, checksum) ) {
    processCmd(command, data);
  }
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void processCmd(String Cmd, String Data) {

    DynamicJsonBuffer  jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    uint16_t command = Cmd.toInt();
    uint16_t data = Data.toInt();

    switch (command) {
      case COMMAND_ACKNOWLEDGE: // COMMAND_ACKNOWLEDGE 500
        json[F("Timestamp")] = getTimeStamp();
        json[F("Command")] = "COMMAND_ACKNOWLEDGE";
        json[F("Value")] = data;
        break;
json.prettyPrintTo(Serial);

      case COMMAND_ERROR: // COMMAND_ERROR 501
        json[F("Timestamp")] = getTimeStamp();
        json[F("Command")] = "COMMAND_ERROR";
        json[F("Value")] = data;
        break;
json.prettyPrintTo(Serial);

      case SYSTEM_ERROR: // SYSTEM_ERROR 502
        json[F("Timestamp")] = getTimeStamp();
        json[F("Command")] = "SYSTEM_ERROR";
        json[F("Value")] = data;
        break;
json.prettyPrintTo(Serial);
                
      case ZONE_ALARM: // ZONE_ALARM 601
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = data;
        json[F("Status")] = "Alarm";
        json[F("Value")] = 2;
        break;
      case ZONE_ALARM_RESTORE: // ZONE_ALARM_RESTORE 602
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = data;
        json[F("Status")] = "Alarm Restore";
        json[F("Value")] = 3;
        break;
      case ZONE_OPEN: // ZONE_OPEN 609
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = data;
        json[F("Status")] = "Open";
        json[F("Value")] = 1;
        sendDSCtoKNX(data, 1);
        break;
      case ZONE_RESTORED: // ZONE_RESTORED 610
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = data;
        json[F("Status")] = "Restore";
        json[F("Value")] = 0;
        break;

      case PARTITION_READY: // PARTITION_READY 650
        json[F("Timestamp")] = getTimeStamp();
        json[F("Partition")] = data;
        json[F("Status")] = "Ready";
        json[F("Value")] = 4;
        break;
      case PARTITION_NOT_READY: // PARTITION_NOT_READY 651
        json[F("Timestamp")] = getTimeStamp();
        json[F("Partition")] = data;
        json[F("Status")] = "Not Ready";
        json[F("Value")] = 5;
        break;
      case PARTITION_ARMED: // PARTITION_ARMED 652
        json[F("Timestamp")] = getTimeStamp();
        json[F("Partition")] = data;
        json[F("Status")] = "Armed";
        json[F("Value")] = 0;
        break;
      case PARTITION_IN_ALARM: // PARTITION_IN_ALARM 654
        json[F("Timestamp")] = getTimeStamp();
        json[F("Partition")] = data;
        json[F("Status")] = "Not Ready";
        json[F("Value")] = 6;
        break;
      case PARTITION_DISARMED: // PARTITION_DISARMED 655
        json[F("Timestamp")] = getTimeStamp();
        json[F("Partition")] = data;
        json[F("Status")] = "Armed";
        json[F("Value")] = 7;
        break;
        
      case TROUBLE_STATUS: // TROUBLE_STATUS 840
        json[F("Timestamp")] = getTimeStamp();
        json[F("Command")] = "TROUBLE_STATUS";
        json[F("Value")] = data;
        break;
      case TROUBLE_STATUS_RESTORE: // TROUBLE_STATUS_RESTORE 841
        json[F("Timestamp")] = getTimeStamp();
        json[F("Command")] = "TROUBLE_STATUS_RESTORE";
        json[F("Value")] = data;
        break;
        
      default:
        if( debug.DSC ) {
          Serial.print(F("Unknown DSC Cmd: "));
          Serial.print(command);
          Serial.print(F(" Data: "));
          Serial.println(data);
        }
        break;
    }
    if( debug.DSC ) {
      json.prettyPrintTo(Serial);
    }
    
    sendMQTT("iot/sariwating/dsc", json);  
}

//--------------------------------------------------------------------------------------------
// setup and send descriptive Arming command
//--------------------------------------------------------------------------------------------
void enableDescriptiveArming() {
  sendDSC(String(DESCRIPTIVE_ARMING_ENABLE) + "1");
}

//--------------------------------------------------------------------------------------------
// setup and send arm command
//--------------------------------------------------------------------------------------------
void dscArmAway() {
  sendDSC(String(PARTITION_ARM_AWAY) + "1");
}

//--------------------------------------------------------------------------------------------
// setup and send arm command
//--------------------------------------------------------------------------------------------
void dscArmStay() {
  sendDSC(String(PARTITION_ARM_ZERO_ENTRY) + "1");
}

//--------------------------------------------------------------------------------------------
// Get Status Report
//--------------------------------------------------------------------------------------------
void dscStatusRequest() { 
  sendDSC(String(STATUS_REPORT));
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void dscPoll() {
  sendDSC(String(POLL));
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void sendDSC(String cmd) {
  cmd = appendChecksum(cmd);
  Serial2.print(cmd);
      Serial.print(F("[DSC] TX ->: "));
      for (int i=0; i<cmd.length();i++){
    Serial.print(cmd[i], HEX);
      }

}
//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
bool DSC_CS_OK( String Command, String Data, String DSC_CS) {

  // Get DSC Generated Checksum
  char buf[3];
  DSC_CS.toCharArray(buf, 3);

  uint8_t cs = strtol(buf, NULL, 16);

  // Serial.println(cs, HEX);

  // Calculate Checksum from Command and Data
  uint16_t csCalc = 0;

  for (int i = 0; i < Command.length(); i++){
    csCalc += Command[i];
  }
  
  for (int i = 0; i < Data.length(); i++){
    csCalc += (char) Data[i];
  }

  uint8_t finalCS = csCalc;

  //Serial.println(finalCS, HEX);

  if( finalCS == cs ) {
    return 1;
  } else {
      if( debug.DSC ) {
      Serial.println(F("DSC Checksum Failed.."));
    }
    return 0;
  }
  
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
String appendChecksum(String str) {
  unsigned char cs = 0;
  for (int n = 0; n < str.length(); ++n)
  {
    cs += (unsigned char)str.charAt(n);
  }
  String csStr = String(cs, HEX);
  csStr.toUpperCase();
  return String(str + csStr + "\r\n");
}
