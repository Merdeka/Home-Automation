#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//--------------------------------------------------------------------------------------------
// Wifi & MQTT Settings
//--------------------------------------------------------------------------------------------
WiFiClient wifi;
WiFiUDP udp; // A UDP instance to let us send and receive packets over UDP

void getNTP();
void sendStatus();

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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
    Serial.println(F("Sent less data than expected!"));
  }

  dataFile.close();
  return true;
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void broadcastWS() {
  
  DynamicJsonBuffer  WSNodeBuffer;
  JsonObject& wsJson = WSNodeBuffer.createObject();

  wsJson[F("VCC")]                  = (float) ESP.getVcc() / 1000;
  wsJson[F("Temperature")]          = (float) sensorData.temperature / 100;
  wsJson[F("WindSpeed_MPS")]        = (float) sensorData.windspeed_mps / 100;
  wsJson[F("WindSpeed_KPH")]        = (float) sensorData.windspeed_kph / 100;
  wsJson[F("WindDirection")]        = windDirectionArray[sensorData.winddirection];
  wsJson[F("WindDirection_Number")] = sensorData.winddirection;
  wsJson[F("ChipID")]               = sensorData.ChipId;
  wsJson[F("ChipSpeed")]            = ESP.getCpuFreqMHz();
  wsJson[F("FlashChipID")]          = sensorData.FlashChipId;
  wsJson[F("FreeRam")]              = ESP.getFreeHeap();
  wsJson[F("Looptime")]             = sensorData.looptime;
  wsJson[F("Uptime")]               = getUptime();

  String ws;
  wsJson.prettyPrintTo(ws);

  webSocket.broadcastTXT(ws);
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void wifiTask() {
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
      Serial.print(F("Connecting to "));
      Serial.print(ssid);
      Serial.println(F("..."));
    #endif
    
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    #ifdef DEBUG
      Serial.println(F("WiFi connected"));
    #endif
    
    getNTP();               // Get NTP Time Update @Wifi Connect

    // Webserver Handlers
    server.on("/list", HTTP_GET, printDirectory);
    server.onNotFound(handleFallBack);

    // Start Webserver
    server.begin();
    Serial.println(F("HTTP server started"));

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  }

  server.handleClient();
  webSocket.loop();
}
