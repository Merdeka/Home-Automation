#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

// Setup TypeDefs
typedef struct { // Sensor Data
    unsigned long looptime;
    uint32_t      ChipId;
    uint32_t      FlashChipId;
} SENSORDATA;
SENSORDATA sensorData;

#include <ArduinoJson.h>
#include "FS.h"
#include "esp_chipid.h"
#include "esp_systemdiag.h"

//--------------------------------------------------------------------------------------------
// Vars
//--------------------------------------------------------------------------------------------
unsigned long fast_update, medium_update, slow_update, ws_update;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char* ssid;
const char* password;

ADC_MODE(ADC_VCC);

bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  ssid     = json["SSID"];
  password = json["PASSWORD"];
  
  return true;
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

void printDirectory() {

  // http://192.168.60.109/list?dir=/
  
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" ) return returnFail("BAD PATH");
  Dir dir = SPIFFS.openDir("/");
  
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();

  while (dir.next()) {

    DynamicJsonBuffer  webBuffer;
    JsonObject& webJson = webBuffer.createObject();
    
    webJson[F("name")]  = dir.fileName();

    String output;
    webJson.printTo(output);
    output += "\n";
    
    server.sendContent(output);
    
 }
}

bool loadFromSPIFFS(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm"))  dataType = "text/html";
  else if(path.endsWith(".css"))  dataType = "text/css";
  else if(path.endsWith(".js"))   dataType = "application/javascript";
  else if(path.endsWith(".png"))  dataType = "image/png";
  else if(path.endsWith(".gif"))  dataType = "image/gif";
  else if(path.endsWith(".jpg"))  dataType = "image/jpeg";
  else if(path.endsWith(".ico"))  dataType = "image/x-icon";
  else if(path.endsWith(".xml"))  dataType = "text/xml";
  else if(path.endsWith(".pdf"))  dataType = "application/pdf";
  else if(path.endsWith(".zip"))  dataType = "application/zip";

  File dataFile = SPIFFS.open(path.c_str(), "r");

  if (!dataFile)
    return false;

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void handleFallBack(){
  if(loadFromSPIFFS(server.uri())) return;

  // File not found on SPIFFS
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.print(message);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void setup() {
  
  Serial.begin(57600);
    Serial.println("");
    delay(1000);
  
    Serial.println("Mounting FS...");
  
    if (!SPIFFS.begin()) {
      Serial.println("Failed to mount file system");
      return;
    }
    
    if (!loadConfig()) {
      Serial.println("Failed to load config");
    } else {
      Serial.println("Config loaded");
  
      Serial.print("Connecting to ");
      Serial.println(ssid);
      
      WiFi.begin(ssid, password);
      
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    
      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }

    // Webserver Handlers
    server.on("/list", HTTP_GET, printDirectory);
    server.onNotFound(handleFallBack);

    // Start Webserver
    server.begin();
    Serial.println("HTTP server started");

     webSocket.begin();
     webSocket.onEvent(webSocketEvent);

    // Print Flash Info
    Serial.print("ChipID: ");
    Serial.println(getChipId());
    Serial.print("FlashChipID: ");
    Serial.println(getFlashChipId());
}

void broadcastWS() {
  
  DynamicJsonBuffer  sensorNodeBuffer;
  JsonObject& nodeJson = sensorNodeBuffer.createObject();

  nodeJson[F("VCC")]            = (float) ESP.getVcc() / 1000;
  nodeJson[F("ChipID")]         = getChipId();
  nodeJson[F("ChipSpeed")]      = ESP.getCpuFreqMHz();
  nodeJson[F("FlashChipID")]    = getFlashChipId();
  nodeJson[F("FreeRam")]        = ESP.getFreeHeap();
  nodeJson[F("Looptime")]       = sensorData.looptime;
  nodeJson[F("Uptime")]         = getUptime();

  String test;
  nodeJson.prettyPrintTo(test);

  webSocket.broadcastTXT(test);
}

void loop(void){
  
  server.handleClient();
  webSocket.loop();

  uptime();     // Runs the uptime routine and reenters the main loop
  loopTimer();  // To Measure the loop time 

  //-------------------------------------------------------------------------------------------
  // Timed Code
  //-------------------------------------------------------------------------------------------
  // Every 250 Milliseconds
  if ((millis()-ws_update)>100) { // 100 Milliseconds

    ws_update = millis();
    broadcastWS();
  }
  // Every 10 Seconds
  if ((millis()-fast_update)>10000) { // 10 Seconds

    fast_update = millis();
  }

  // Every 30 Seconds
  if ((millis()-medium_update)>30000) { // 30 Seconds
      
    medium_update = millis();
  }

  // Every 60 minutes
  if ((millis()-slow_update)>36000000) { // 60 minutes
      
    slow_update = millis();
  }

}
