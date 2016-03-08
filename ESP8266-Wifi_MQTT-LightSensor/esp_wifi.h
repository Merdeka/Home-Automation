#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

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
      Serial.print("Connecting to ");
      Serial.print(ssid);
      Serial.println("...");
    #endif
    
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    #ifdef DEBUG
      Serial.println("WiFi connected");
    #endif
    
    getNTP();               // Get NTP Time Update @Wifi Connect
  }
}
