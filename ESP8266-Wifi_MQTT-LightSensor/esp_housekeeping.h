#define DEBUG

// Setup TypeDefs
typedef struct { // Sensor Data
    uint16_t      lightSensor;
    uint16_t      temperature;
    uint16_t      humidity;
    unsigned long looptime;
    uint32_t      ChipId;
    uint32_t      FlashChipId;
} SENSORDATA;
SENSORDATA sensorData;

#include "esp_wifi.h"
#include "esp_timefunctions.h"
#include "esp_chipid.h"
#include "esp_systemdiag.h"

ADC_MODE(ADC_VCC);

//--------------------------------------------------------------------------------------------
// Convert MAC address to String
//--------------------------------------------------------------------------------------------
String macToStr(const uint8_t* mac) {
  
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

String displayIP(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}
