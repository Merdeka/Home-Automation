#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "ESP8266-OLED"

//--------------------------------------------------------------------------------------------
// Wifi & MQTT Settings
//--------------------------------------------------------------------------------------------
const char* ssid = "Timo & Roef";
const char* password = "password";

WiFiClient wifi;
WiFiUDP udp; // A UDP instance to let us send and receive packets over UDP

void getNTP();
void sendStatus();

void wifiTask() {
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
      Serial.print(F("Connecting to "));
      Serial.print(ssid);
      Serial.println(F("..."));
    #endif

    WiFi.hostname(HOSTNAME);
    
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    #ifdef DEBUG
      Serial.println(F("WiFi connected"));
    #endif
    
    wifiStatus = "Connected";
    getNTP();               // Get NTP Time Update @Wifi Connect

    // Start OTA server.
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.begin();
  } else {
    // Handle OTA server.
    ArduinoOTA.handle();
    yield();
  }
}
