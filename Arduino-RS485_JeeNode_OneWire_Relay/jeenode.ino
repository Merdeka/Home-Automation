#define MYNODE  30            //Should be unique on network, node ID 30 reserved for base station
#define FREQ    RF12_868MHZ   //frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define GROUP   30            //network group, must be same as emonTx and emonBase

void setupJeeNode() {

    #ifdef DEBUG
      Serial.print(F("Starting JeeNode NodeID: "));
      Serial.print(MYNODE);
      Serial.print(F(" Group: "));
      Serial.println(GROUP);
    #endif
    
    rf12_initialize(MYNODE, FREQ, GROUP);
    rf12_sleep(RF12_SLEEP);
}

void jeeNodeTask() {
  
    if (rf12_recvDone()){
      if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
      {
       uint8_t node_id = (rf12_hdr & 0x1F);

       
       char buf[2];
       sprintf( buf, "%02i", node_id );

       int addrCmd = strtol(buf, NULL, 16);

       #ifdef DEBUG
        Serial.print("Relaying RF12B JeeNode: ");
        Serial.println(addrCmd, HEX);
       #endif

       ICSC.broadcast(char(addrCmd), rf12_len, (char *)rf12_data );
      }
    }
}

void JeeRelay(unsigned char src, char command, unsigned char len, char *data) {
   
    rf12_sleep(RF12_WAKEUP);
    rf12_sendNow(0, (void *)data, len);
    rf12_sendWait(2);
    rf12_sleep(RF12_SLEEP);

    #ifdef DEBUG
      Serial.print("Relaying RS485 Node: ");
      Serial.println(src, HEX);
    #endif
}
