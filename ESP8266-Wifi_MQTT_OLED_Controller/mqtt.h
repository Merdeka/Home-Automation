#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT Server
IPAddress mqtt_server(192, 168, 61, 15);

PubSubClient mqtt(wifi);

long lastReconnectAttempt = 0;
long mqttConnected;

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

      // Once connected
      sendStatus();           // Send Out a Status Report @MQTT Connect
      
      #ifdef DEBUG
        Serial.println(F("Connected"));
      #endif
      
      mqttStatus = "Connected";
      mqttConnected = millis();
      getNTP();
      // ... and resubscribe
      mqtt.subscribe("iot/sariwating/onewire");
      mqtt.subscribe("iot/sariwating/audio");
      mqtt.subscribe("iot/sariwating/hardware/status/soladinnode");

      char* dev = "OLED";
      mqtt.publish("iot/sariwating/audio/get", dev);
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

   if( String((char*)topic) == F("iot/sariwating/onewire")) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);
      
      String address = json["Address"];
      if(address == "10:4a:b2:4b:01:08:00:24") {
        String temp = json["Temperature"];
        INSIDETEMPERATURE = temp;
      }
   } else if( String((char*)topic) == F("iot/sariwating/audio")) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);

      String state = json["State"];
      if (state == "pause") {
        PLAYSTATE = false;
        
      } else if (state == "play") {
        PLAYSTATE = true;
        
        String artist = json["Artist"];
        ARTIST = artist;
        String title = json["Title"];
        TITLE = title;
        
        VOLUME = json["Volume"];
        uint16_t duration = json["Duration"];
        uint16_t songtime = json["Time"];
        if(duration == 0) {
          SONGSTATUSBAR = false;
        } else {
          SONGDURATION = duration / 60;
          SONGDURATION += ":";
          SONGDURATION += printDigits(duration % 60);
  
          SONGTIME = songtime / 60;
          SONGTIME += ":";
          SONGTIME += printDigits(songtime % 60);
  
          SONGPERCENTAGE = ((float) songtime / (float) duration) * 100;
          
          SONGSTATUSBAR = true;
        }
      }
      
   } else if( String((char*)topic) == F("iot/sariwating/hardware/status/soladinnode")) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);
      
      String temp = json["Temperature"];
      OUTSIDETEMPERATURE = temp;
   }
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
  nodeJson[F("Looptime")]       = (float) sensorData.looptime / 1000;
  nodeJson[F("Uptime")]         = getUptime();
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/hardware/status/oled", nodeJson);
}

void sendVolume(uint8_t volume) {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();

  nodeJson[F("Timestamp")]    = getTimeStamp(); 
  nodeJson[F("Cmd")]     = "Volume"; 
  nodeJson[F("Value")]   = volume; 
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/audio/set", nodeJson);
}

void sendAudioState(uint8_t state) {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();

  nodeJson[F("Cmd")]     = "State"; 
  nodeJson[F("Value")]   = state; 
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/audio/set", nodeJson);
}

void sendKNX(String address, bool Value) {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();

  nodeJson[F("Timestamp")]    = getTimeStamp(); 
  nodeJson[F("Address")] = address; 
  nodeJson[F("Value")]   = (uint8_t) Value; 
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/knx/set", nodeJson);
}

void sendButton(uint8_t button) {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();

  nodeJson[F("Timestamp")]    = getTimeStamp(); 
  nodeJson[F("Cmd")]     = "Favorite"; 
  nodeJson[F("Value")]   = button; 
   
  #ifdef DEBUG
    nodeJson.prettyPrintTo(Serial);
  #endif

  sendMQTT("iot/sariwating/audio/set", nodeJson);
}

