#include <JeeLib.h>

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 25 // Should be unique on network, node ID 30 reserved for base station
#define FREQ RF12_868MHZ // frequency - match to same frequency as RFM12B module
#define GROUP 30 // network group, must be same as emonTx and emonBase

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
