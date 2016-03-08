#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port Analog 0 on the Arduino
#define ONE_WIRE_BUS 14
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire ONEWIRE(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&ONEWIRE);
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

uint8_t numberOfDevices; // Number of temperature devices found

void sendOneWireReport() {

    // Store the Local Node Temperature
    char* address = "00:00:00:00:00:00:00:00";
    sprintf(address,"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", oneWire.address0, oneWire.address1, oneWire.address2, oneWire.address3, oneWire.address4, oneWire.address5, oneWire.address6, oneWire.address7);

    const char* OneWireJeeNode  PROGMEM = "10:5f:eb:ed:00:08:00:f9";

    if( (String) address == (String) OneWireJeeNode ) { // OneWire JeeNode Gateway Temp
      oneWireNode.Temp = oneWire.temperature;
    }

    // Send the Report
    char buffer[sizeof oneWire];
    memcpy(buffer, &oneWire, sizeof oneWire);
    
    ICSC.broadcast(char(0x31), sizeof(buffer), buffer);
    
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

void sendSensorsReport() {

    oneWireNode.uptimeDay    = uptimeDay;
    oneWireNode.uptimeHour   = uptimeHour;
    oneWireNode.uptimeMinute = uptimeMinute;
    oneWireNode.uptimeSecond = uptimeSecond;
    oneWireNode.freeRam      = freeRam();
  
    char buffer[sizeof oneWireNode];
    memcpy(buffer, &oneWireNode, sizeof oneWireNode);   
    ICSC.broadcast(char(0x32), sizeof(oneWireNode), buffer);

    #ifdef ONEWIRE_DEBUG
      Serial.println(F("Sending Sensors Report"));
      Serial.print(F("Temperature: "));
      Serial.print((float) oneWireNode.Temp / 100);
      Serial.print(F(" Uptime: "));
      Serial.print(getUptime());
      Serial.print(F(" Free Ram: "));
      Serial.println(freeRam());
    #endif
    
    #if SERIAL
        SPrintDS();
        serialFlush();
    #endif
}

void setupOnewire() {
  
  // Start up the library
  sensors.begin();
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
    for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
  {
    // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
        }
  }
  
}

void onewireTask() {
  
    sensors.requestTemperatures(); // Send the command to get temperatures
    
    // Loop through each device, print out temperature data
    for(int i=0;i<numberOfDevices; i++)
    {
      // Search the wire for address
      if(sensors.getAddress(tempDeviceAddress, i)) {
        
        oneWire.address0 = tempDeviceAddress[0];
        oneWire.address1 = tempDeviceAddress[1];
        oneWire.address2 = tempDeviceAddress[2];
        oneWire.address3 = tempDeviceAddress[3];
        oneWire.address4 = tempDeviceAddress[4];
        oneWire.address5 = tempDeviceAddress[5];
        oneWire.address6 = tempDeviceAddress[6];
        oneWire.address7 = tempDeviceAddress[7];
        oneWire.temperature = (sensors.getTempC(tempDeviceAddress)) * 100;

        #ifdef ONEWIRE_DEBUG
          if (oneWire.address0 < 16) Serial.print("0");
            Serial.print(oneWire.address0, HEX);
          if (oneWire.address1 < 16) Serial.print("0");
            Serial.print(oneWire.address1, HEX);
          if (oneWire.address2 < 16) Serial.print("0");
            Serial.print(oneWire.address2, HEX);
          if (oneWire.address3 < 16) Serial.print("0");
            Serial.print(oneWire.address3, HEX);
          if (oneWire.address4 < 16) Serial.print("0");
            Serial.print(oneWire.address4, HEX);
          if (oneWire.address5 < 16) Serial.print("0");
            Serial.print(oneWire.address5, HEX);
          if (oneWire.address6 < 16) Serial.print("0");
            Serial.print(oneWire.address6, HEX);
          if (oneWire.address7 < 16) Serial.print("0");
            Serial.print(oneWire.address7, HEX);
            Serial.print(" - ");
            Serial.println((float) oneWire.temperature / 100 );
        #endif
      }

      sendOneWireReport();
    }
}
