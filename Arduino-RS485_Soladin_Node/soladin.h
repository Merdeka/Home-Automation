#include <Soladin.h>

Soladin sol;                  // copy of soladin class
bool connectSol = 0 ;         // if soladin responds

SoftwareSerial solcom(14, 4); // serial to conect to soladin1 (AIO1 TX, DIO1 RX)

void doFW(){
  if (connectSol)  {              // already connected
    for (int i=0 ; i < 4 ; i++) {     // give me some time to return by hand
      if (sol.query(FWI)) {       // request firware information
        break; 
      } 
    }
  }
}

void doDS(){
  if (connectSol) {
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(DVS)) {       // request Device status       
        break;
      }
    } 
  }
}

void doSoladinMeasure(){
  
  if (!connectSol) {                    // Try to connect
    
    solcom.begin(9600);
    sol.begin(&solcom);
    
    Serial.println(F("Soladin Probe..."));
    for (int i=0 ; i < 4 ; i++) {
      if (sol.query(PRB)) {       // Try connecting to slave
        connectSol = true;
        Serial.println(F("Soladin connected"));
        delay(50);
        sol.PVvolt = 0;
        sol.PVamp = 0;
        sol.Gridpower = 0;
        sol.Gridfreq = 0;
        sol.Gridvolt = 0;
        sol.DeviceTemp = 0;
        sol.Flag = 0;
        sol.Totalpower = 0;
        sol.TotalOperaTime = 0;
  
        doDS();
 
        soladin.PVvolt              = sol.PVvolt;
        soladin.PVamp               = sol.PVamp;
        soladin.ACgridPower         = sol.Gridpower;
        soladin.ACgridFreq          = sol.Gridfreq;
        soladin.ACgridVolt          = sol.Gridvolt;
        soladin.DeviceTemp          = sol.DeviceTemp;
        soladin.AlarmFlag           = sol.Flag;
        soladin.TotalPower          = sol.Totalpower;
        soladin.TotalOperationTime  = sol.TotalOperaTime;     
  
        Serial.print(F("Soladin: "));
        Serial.print(soladin.PVvolt);
        Serial.print(F(" Volt, "));
        Serial.print(soladin.PVamp);
        Serial.print(F(" Amp, "));
        Serial.print(soladin.ACgridPower);
        Serial.println(F(" Watt"));

        solcom.end();
        connectSol = false;
        break;
      }
    }
  }
}
