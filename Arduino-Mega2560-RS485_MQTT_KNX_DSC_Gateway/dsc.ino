#include "dsccommands.h"

String rxBuffer;

void setupDSC() {
  
  Serial2.begin(9600);

  enableDescriptiveArming();
  dscStatusRequest();
}

void stopDSC() {
  
  Serial2.end();
}

void enableDSC() {
  if( debug.MAIN ) { Serial.println(F("DSC Protocol Enabled")); }
  interfaces.DSC = true;
  setupDSC();
}

void disableDSC() {
  if( debug.MAIN ) { Serial.println(F("DSC Protocol Disabled")); }
  interfaces.DSC = false;
  stopDSC();
}

void dscTask() {

  while ( Serial2.available() > 0 ) {

    char data = Serial2.read();

      // Process message when line feed character is recieved
      if (data == (char) 0x0D )
      {
        rxBuffer.trim();           
        processCmd(rxBuffer);
  
        rxBuffer = ""; // clear RX Buffer
 /*     } else if (data == (char) 0x0A ) {
        uint8_t CR = rxBuffer.length() - 2;
          if (rxBuffer.charAt(CR) == (char) 0x0D) {
            
            rxBuffer.trim();           
            processCmd(rxBuffer);
    
            rxBuffer = ""; // clear RX Buffer
          } else {
            rxBuffer = ""; // clear RX Buffer
          }*/
       } else {
        rxBuffer += data;
       }
  }
}

void processCmd(String str) {

    DynamicJsonBuffer  jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    uint16_t command = str.substring(0,3).toInt();

    switch (command) {
      case ZONE_ALARM: // zone alarm 601
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = str.substring(3, 6).toInt();
        json[F("Status")] = "Alarm";
        json[F("Value")] = 2;
        break;
      case ZONE_ALARM_RESTORE: // zone alarm restored 602
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = str.substring(3, 6).toInt();
        json[F("Status")] = "Alarm Restore";
        json[F("Value")] = 3;
        break;
      case ZONE_OPEN: // zone open 609
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = str.substring(3, 6).toInt();
        json[F("Status")] = "Open";
        json[F("Value")] = 1;
        break;
      case ZONE_RESTORED: // zone restored 610
        json[F("Timestamp")] = getTimeStamp();
        json[F("Zone")] = str.substring(3, 6).toInt();
        json[F("Status")] = "Restore";
        json[F("Value")] = 0;
        break;
      default:
        if( debug.DSC ) {
          Serial.print(F("Unknown DSC Cmd: "));
          Serial.print(command);
          Serial.print(F(" Data: "));
          Serial.println(str);
        }
        break;
    }
    if( debug.DSC ) {
      json.prettyPrintTo(Serial);
      Serial.println(str);
    }
    
    sendMQTT("iot/sariwating/dsc", json);  
}

void enableDescriptiveArming() {
   // setup and send descriptive Arming command
  String cmd = String(DESCRIPTIVE_ARMING_ENABLE);
  cmd += "1";
  cmd = appendChecksum(cmd);
  Serial2.print(cmd);
  if( debug.DSC ) {
    Serial.print(cmd);
  }  
}

void dscArmAway() {
  // setup and send arm command
  String cmd = String(PARTITION_ARM_AWAY);
  cmd += "1";
  cmd = appendChecksum(cmd);
  Serial2.print(cmd);
  if( debug.DSC ) { Serial.print(cmd); } 
}

void dscArmStay() {
  // setup and send arm command
  String cmd = String(PARTITION_ARM_ZERO_ENTRY);
  cmd += "1";
  cmd = appendChecksum(cmd);
  Serial2.print(cmd);
  if( debug.DSC ) { Serial.print(cmd); }  
}

void dscStatusRequest() {
  String cmd = String(STATUS_REPORT);
  cmd = appendChecksum(cmd);
  Serial2.print(cmd);
  if( debug.DSC ) { Serial.print(cmd); }
}

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
