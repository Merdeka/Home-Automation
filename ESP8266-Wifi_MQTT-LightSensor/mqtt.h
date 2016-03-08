#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT Server
IPAddress mqtt_server(192, 168, 61, 15);

PubSubClient mqtt(wifi);

long lastReconnectAttempt = 0;

boolean mqttReconnect() {

    // Generate client name based on MAC address and last 8 bits of microsecond counter
    String clientName;
    clientName += "esp8266-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    clientName += "-";
    clientName += String(micros() & 0xff, 16);
    
    if (mqtt.connect((char*) clientName.c_str())) {
      sendStatus();           // Send Out a Status Report @MQTT Connect
      #ifdef DEBUG
        Serial.println("connected");
      #endif
      // Once connected, publish an announcement...
      
      // ... and resubscribe
    }
  return mqtt.connected();
}

void sendMQTT(const char* topic, JsonObject& sendJson) {
  
  if (mqtt.connected()) {
    char buffer[sendJson.measureLength() + 1];
    sendJson.printTo(buffer, sizeof(buffer));
    mqtt.publish(topic, buffer);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  
}

void setupMQTT() {
  
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(mqttCallback);
    lastReconnectAttempt = 0;
}

void mqttTask() {
  
    if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) {
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (mqttReconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      
      // MQTT connected
      mqtt.loop();
      }
    }
}

void sendSensors() {

  DynamicJsonBuffer  sensorBuffer;
  JsonObject& sensorJson = sensorBuffer.createObject();
  
  sensorJson[F("Timestamp")]    = getTimeStamp(); 
  sensorJson[F("Temperature")]  = (float) sensorData.temperature / 100;
  sensorJson[F("Humidity")]     = (float) sensorData.humidity / 100;
  sensorJson[F("LightSensor")]  = sensorData.lightSensor;

  #ifdef DEBUG
    sensorJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/esp/lightsensor", sensorJson);
}

void sendStatus() {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();
  
  nodeJson[F("Timestamp")]      = getTimeStamp(); 
  nodeJson[F("IP")]             = displayIP(WiFi.localIP());
  nodeJson[F("VCC")]            = (float) ESP.getVcc() / 1000;
  nodeJson[F("ChipID")]         = getChipId();
  nodeJson[F("ChipSpeed")]      = ESP.getCpuFreqMHz();
  nodeJson[F("FlashChipID")]    = getFlashChipId();
  nodeJson[F("FreeRam")]        = ESP.getFreeHeap();
  nodeJson[F("Uptime")]         = getUptime();
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/hardware/status/lightsensor", nodeJson);
}

