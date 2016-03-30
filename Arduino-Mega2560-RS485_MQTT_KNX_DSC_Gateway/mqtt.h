// Remember to set #define MQTT_MAX_PACKET_SIZE 512 in PubSubClient.h!!!

#include <PubSubClient.h>

PubSubClient mqtt(ETHERNET);

IPAddress mqtt_server(192, 168, 61, 15);
const uint8_t MQTTLED = 28;
long lastReconnectAttempt = 0;

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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
  sensorJson[F("Looptime")]     = sensors.looptime;
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

  JsonObject& io = sensorJson.createNestedObject("IO");
  io[F("RS485BUS")]             = interfaces.RS485BUS;
  
  if( debug.LOCALSENSORS ) { sensorJson.prettyPrintTo(Serial); }

  sendMQTT("iot/sariwating/hardware/status/gatewaynode", sensorJson);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
boolean mqttReconnect() {    
    if (mqtt.connect("arduinoClient")) {
      getNTP();               // Get NTP Time @MQTT Connect
      sendStatus();           // Send Out a Status Report @MQTT Connect
      
      if( debug.MAIN ) { Serial.println("MQTT connected"); }
      // Once connected, publish an announcement...
      
      // ... and resubscribe
      mqtt.subscribe("iot/sariwating/knx/set");
      mqtt.subscribe("iot/sariwating/rfxcom/x10/json");
      mqtt.subscribe("iot/sariwating/shednode/set");

      if ( interfaces.DSC ) {
        dscStatusRequest();
      }
    }
  return mqtt.connected();
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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
      if( address == "0/0/9" ) {
        bool value = json["Value"];
        Serial.println(value);
        Knx.write(4, (byte) value);
        
        DynamicJsonBuffer  rfxcomBuffer;
        JsonObject& rfxcomJson = rfxcomBuffer.createObject();

        String CMD;

        rfxcomJson[F("Timestamp")]      = getTimeStamp();
        rfxcomJson[F("Device")]         = "AC";
        rfxcomJson[F("Address")]        = "0x0042215B";
        rfxcomJson[F("Unitcode")]       = 3;
        if(value == 1) { CMD = "On"; } else if(value == 0) { CMD = "OFF"; } 
        rfxcomJson[F("Command")]        = CMD;
  
        sendMQTT("iot/sariwating/rfxcom/set", rfxcomJson);
        
      } else if( address == "0/5/1" ) {
        bool value = json["Value"];
        Serial.println(value);
        Knx.write(5, (byte) value);
      } else if( address == "0/5/2" ) {
        bool value = json["Value"];
        Serial.println(value);
        Knx.write(6, (byte) value);
      } else if( address == "0/0/10" ) {
        bool value = json["Value"];
        Serial.println(value);
        Knx.write(7, (byte) value);
      }
    }  
  }
  
  else if( String((char*)topic) == F("iot/sariwating/rfxcom/x10/json") ) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject((char*)payload);

    String device = json["Device"];

    if(device == "X10") {
      String housecode = json["Housecode"];
      uint8_t unitcode = json["Unitcode"];
      String command = json["Command"];
      
      if(housecode == "A" && unitcode == 1) {   
        if(command =="On" && interfaces.KNX) {
          Knx.write(6, 1);
        }
      } else if(housecode == "H" && unitcode == 1) {
          uint8_t cmd;
          if(command =="On"){
            cmd = 1;
          } else if (command == "Off") {
            cmd = 0;
          }
          Knx.write(7, cmd);
      } else if(housecode == "H" && unitcode == 2) {
          DynamicJsonBuffer  rfxcomBuffer;
          JsonObject& rfxcomJson = rfxcomBuffer.createObject();
          
          rfxcomJson[F("Timestamp")]      = getTimeStamp();
          rfxcomJson[F("Device")]         = "AC";
          rfxcomJson[F("Address")]        = "0x0042215B";
          rfxcomJson[F("Unitcode")]       = 2;
          rfxcomJson[F("Command")]        = command;
  
          sendMQTT("iot/sariwating/rfxcom/set", rfxcomJson);
          
      } else if(housecode == "H" && unitcode == 3) {
          DynamicJsonBuffer  rfxcomBuffer;
          JsonObject& rfxcomJson = rfxcomBuffer.createObject();
          
          rfxcomJson[F("Timestamp")]      = getTimeStamp();
          rfxcomJson[F("Device")]         = "AC";
          rfxcomJson[F("Address")]        = "0x0042215B";
          rfxcomJson[F("Unitcode")]       = 3;
          rfxcomJson[F("Command")]        = command;
  
          sendMQTT("iot/sariwating/rfxcom/set", rfxcomJson);
          
          uint8_t cmd;
          if(command =="On"){
            cmd = 1;
          } else if (command == "Off") {
            cmd = 0;
          }
          Knx.write(4, cmd);
      }
    }
  }

  else if( String((char*)topic) == F("iot/sariwating/shednode/set") ) {

    if( interfaces.RS485 ) {
      
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject((char*)payload);
  
      uint8_t relay = json["Relay"];
      Serial.print(relay);
      Serial.print(" ");
      bool value = json["Value"];
      Serial.println(value);

      switch (relay) {
        case 1:
          shedIO.relay_1 = value;
          break;
        case 2:
          shedIO.relay_2 = value;
          break;
      }

      char buffer[sizeof shedIO];
      memcpy(buffer, &shedIO, sizeof shedIO); 
      sendRS485(char(0x45), sizeof(shedIO), buffer);
    }  
  }
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void sendMQTT(const char* topic, JsonObject& sendJson) {
  
  if (mqtt.connected()) {
    char buffer[sendJson.measureLength() + 1];
    sendJson.printTo(buffer, sizeof(buffer));
    mqtt.publish(topic, buffer);
  }
  
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void setupMQTT() {

  // MQTT Status LED
  pinMode(MQTTLED, OUTPUT);
  digitalWrite(MQTTLED, HIGH);
    
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(mqttCallback);
  lastReconnectAttempt = 0;
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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
