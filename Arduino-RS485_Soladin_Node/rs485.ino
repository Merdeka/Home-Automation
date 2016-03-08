const unsigned char NodeID  = 33; //Node ID 33 on RS485 Network
const uint8_t DEPin         = 16;
const long Baud             = 57600;

SoftwareSerial rs485(6, 5); // RX, TX

void setupRS485() {
    Serial.println(F("Starting RS485"));
    ICSC.begin(NodeID, Baud, &rs485, DEPin);
}

void sendSoladinReport() {
  
    char buffer[sizeof soladin];
    memcpy(buffer, &soladin, sizeof soladin);   
    ICSC.broadcast(char(0x23), sizeof(soladin), buffer);
   
    Serial.println(F("Sending Soladin Report"));
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

void sendNodeReport() {

    soladinNode.uptimeDay    = uptimeDay;
    soladinNode.uptimeHour   = uptimeHour;
    soladinNode.uptimeMinute = uptimeMinute;
    soladinNode.uptimeSecond = uptimeSecond;
    soladinNode.freeRam      = freeRam();
  
    char buffer[sizeof soladinNode];
    memcpy(buffer, &soladinNode, sizeof soladinNode);   
    ICSC.broadcast(char(0x24), sizeof(soladinNode), buffer);
   
    Serial.println(F("Sending Node Report"));
    Serial.print(F("Uptime: "));
    Serial.print(getUptime());
    Serial.print(F(" Free Ram: "));
    Serial.println(freeRam());
    
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

