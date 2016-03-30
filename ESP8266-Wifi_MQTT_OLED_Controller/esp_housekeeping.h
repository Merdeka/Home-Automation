#define DEBUG

// Modes
#define settingsMode  0
#define knxMode       1
#define audioMode     2
#define tempMode      3
#define weatherMode   4

#define modeCount 3      // This is the number of modes to cycle through. There are other "hidden" modes, settings / diags.

uint8_t currentMode = settingsMode;         // default mode
uint8_t storedMode = knxMode;

String ARTIST = "Artist";
String TITLE = "Track";
uint8_t VOLUME = 45;
bool PLAYSTATE = true;
String SONGTIME;
String SONGDURATION;
uint8_t SONGPERCENTAGE;
bool SONGSTATUSBAR = false;

String INSIDETEMPERATURE, OUTSIDETEMPERATURE;

bool SETTINGS = false;

// Setup TypeDefs
typedef struct { // Sensor Data
    unsigned long looptime;
    uint32_t      ChipId;
    uint32_t      FlashChipId;
} SENSORDATA;
SENSORDATA sensorData;

String wifiStatus = "Connecting";
String mqttStatus = "Disconnected";

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
