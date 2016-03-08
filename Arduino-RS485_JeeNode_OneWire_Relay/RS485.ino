unsigned char NodeID  = 30; //Node ID 30 on RS485 Network
const int DEPin       = 5;
const long Baud       = 57600;

AltSoftSerial RS485;

void setupRS485() {

    #ifdef DEBUG
      Serial.print(F("Starting RS485 NodeID: "));
      Serial.println(NodeID);
    #endif
    
    ICSC.begin(NodeID, Baud, &RS485, DEPin);
    ICSC.registerCommand(char(0x30), &JeeRelay);
}
