// Remember to set #define MQTT_MAX_PACKET_SIZE 512 in PubSubClient.h!!!

#include <PubSubClient.h>

PubSubClient mqtt(ethClient);

IPAddress mqtt_server(192, 168, 61, 15);
const uint8_t MQTTLED = 28;
long lastReconnectAttempt = 0;

void sendStatus() {

  DynamicJsonBuffer  sensorBuffer;
  JsonObject& sensorJson = sensorBuffer.createObject();
  
  sensorJson[F("Timestamp")]    = getTimeStamp(); 
  sensorJson[F("Temperature")]  = (float) sensors.temperature / 100;
  sensorJson[F("Humidity")]     = sensors.humidity;
  sensorJson[F("Barometer")]    = (float) sensors.barometer / 100;
  sensorJson[F("Voltage")]      = sensors.voltage;
  sensorJson[F("Current")]      = sensors.current;
  sensorJson[F("IP")]           = displayIP(Ethernet.localIP());
  sensorJson[F("FreeRam")]      = freeRam();
  sensorJson[F("Uptime")]       = getUptime();
  
  JsonObject& protocols = sensorJson.createNestedObject("Protocols");
  protocols[F("RS485")]         = interfaces.RS485;
  protocols[F("KNX")]           = interfaces.KNX;
  protocols[F("DSC")]           = interfaces.DSC;

  JsonObject& debugs = sensorJson.createNestedObject("Debug");
  debugs[F("MAIN")]             = debug.MAIN;
  debugs[F("ETH")]              = debug.ETH;
  debugs[F("MQTT")]             = debug.MQTT;
  debugs[F("KNX")]              = debug.KNX;
  debugs[F("RS485")]            = debug.RS485;
  debugs[F("DSC")]              = debug.DSC;
  debugs[F("SOLADIN")]          = debug.SOLADIN;
  debugs[F("ONEWIRE")]          = debug.ONEWIRE;
  debugs[F("POWER")]            = debug.POWER;
  debugs[F("PLANTNODE")]        = debug.PLANTNODE;
  debugs[F("SENSORS")]          = debug.LOCALSENSORS;
  
  if( debug.LOCALSENSORS ) { sensorJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/hardware/status/gatewaynode", sensorJson);
}

boolean mqttReconnect() {    
    if (mqtt.connect("arduinoClient")) {
      getNTP();               // Get NTP Time @MQTT Connect
      sendStatus();           // Send Out a Status Report @MQTT Connect
      
      if( debug.MAIN ) { Serial.println("MQTT connected"); }
      // Once connected, publish an announcement...
      
      // ... and resubscribe
      mqtt.subscribe("iot/sariwating/knx/set");
    }
  return mqtt.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  
  // handle message arrived
  
  if( debug.MQTT ) {
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));
    for (int i=0;i<length;i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }

  if( String((char*)topic) == F("iot/sariwating/knx/set") ) {

    if( interfaces.KNX ) {
      
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);
  
      String address = json["Address"];
      Serial.print(address);
      Serial.print(" ");
      if( address == "2/0/1" ) {
        bool value = json["Value"];
        Serial.println(value);
        Knx.write(4, (byte) value);
      }
    }  
  }
}

void sendMQTT(const char* topic, JsonObject& sendJson) {
  
  if (mqtt.connected()) {
    char buffer[sendJson.measureLength() + 1];
    sendJson.printTo(buffer, sizeof(buffer));
    mqtt.publish(topic, buffer);
  }
  
}

void setupMQTT() {

  // MQTT Status LED
  pinMode(MQTTLED, OUTPUT);
  digitalWrite(MQTTLED, HIGH);
    
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(mqttCallback);
  lastReconnectAttempt = 0;
}

void mqttTask() {
  
  if (!mqtt.connected()) {
    digitalWrite(MQTTLED, HIGH);
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
    
    if(MQTTLED != LOW) {
      digitalWrite(MQTTLED, LOW);
    }
  }
}
