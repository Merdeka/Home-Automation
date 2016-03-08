#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>;
#include <SD.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   20

#define TCP_BUFFER_SIZE    128
static char buf[TCP_BUFFER_SIZE];  

EthernetClient ethClient;
EthernetUDP udp;
EthernetServer webServer(80);  // create a server at port 80

File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip    ( 192, 168, 60,  220 );
IPAddress gw    ( 192, 168, 60,  254 );
IPAddress mask  ( 255, 255, 255, 0   );

void setupEthernet() {
  
  // Setup Ethernet
  Ethernet.begin(mac, ip, dns, gw, mask);
  // Allow the hardware to sort itself out
  delay(1500);
  
  // initialize SD card
  if( debug.MAIN ) { Serial.println(F("Initializing SD card..")); }
  if (!SD.begin(4)) {
    if( debug.MAIN ) { Serial.println(F("ERROR - SD card initialization failed!")); }
      return;    // init failed
  }
  if( debug.MAIN ) { Serial.println(F("SUCCESS - SD card initialized.")); }
  // check for index.html file
  if (!SD.exists(F("index.htm"))) {
      if( debug.MAIN ) { Serial.println(F("ERROR - Can't find index.htm file!")); }
      return;  // can't find index file
  }
  if( debug.MAIN ) { Serial.println(F("SUCCESS - Found index.htm file.")); }

  webServer.begin();           // start to listen for clients
}

String displayIP(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}

String getUptime();

// send the state update to the web browser
void getjSonState(EthernetClient cl)
{ 
  DynamicJsonBuffer  webBuffer;
  JsonObject& webJson = webBuffer.createObject();

  webJson[F("Timestamp")]    = getTimeStamp(); 
  webJson[F("Temperature")]  = (float) sensors.temperature / 100;
  webJson[F("Humidity")]     = sensors.humidity;
  webJson[F("Barometer")]    = (float) sensors.barometer / 100;
  webJson[F("FreeRam")]      = freeRam();
  webJson[F("Looptime")]     = (float) sensors.looptime / 1000;
  webJson[F("Uptime")]       = getUptime();

  JsonObject& protocols = webJson.createNestedObject("Protocols");
  protocols[F("RS485")]         = interfaces.RS485;
  protocols[F("KNX")]           = interfaces.KNX;
  protocols[F("DSC")]           = interfaces.DSC;

  JsonObject& debugs = webJson.createNestedObject("Debug");
  debugs[F("MAIN")]          = debug.MAIN;
  debugs[F("ETH")]           = debug.ETH;
  debugs[F("MQTT")]          = debug.MQTT;
  debugs[F("KNX")]           = debug.KNX;
  debugs[F("RS485")]         = debug.RS485;
  debugs[F("DSC")]           = debug.DSC;
  debugs[F("SOLADIN")]       = debug.SOLADIN;
  debugs[F("ONEWIRE")]       = debug.ONEWIRE;
  debugs[F("POWER")]         = debug.POWER;
  debugs[F("PLANTNODE")]     = debug.PLANTNODE;
  debugs[F("SENSORS")]       = debug.LOCALSENSORS;
   
  if( debug.ETH ) { webJson.prettyPrintTo(Serial); }
     
  webJson.printTo(cl);
}


char* processFile(char clientline[255]) {
   char *filename;
   filename = clientline + 5;
  (strstr(clientline, " HTTP"))[0] = 0;
  return filename;
}

void webServerTask() {

    bool DONOTPRINT = false;

    EthernetClient client = webServer.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header

                    if (StrContains(HTTP_req, "GET / ")
                                 || StrContains(HTTP_req, "GET /index.htm")) {
                        webFile = SD.open(F("index.htm"));        // open web page file
                        if (webFile) {
                          client.println(F("HTTP/1.1 200 OK"));
                          client.println(F("Content-Type: text/html"));
                          client.println(F("Connnection: close"));
                          client.println();
                        }
                    }

                    else if (StrContains(HTTP_req, "GET /arduino.png")) {
                        webFile = SD.open(F("arduino.png"));
                        if (webFile) {
                            client.println(F("HTTP/1.1 200 OK"));
                            client.println();
                        }
                    }

                    else if (StrContains(HTTP_req, "GET /oslogo.png")) {
                        webFile = SD.open(F("oslogo.png"));
                        if (webFile) {
                            client.println(F("HTTP/1.1 200 OK"));
                            client.println();
                        }
                    }

                    else if (StrContains(HTTP_req, "GET /bg.jpg")) {
                        webFile = SD.open(F("bg.jpg"));
                        if (webFile) {
                            client.println(F("HTTP/1.1 200 OK"));
                            client.println();
                        }
                    }
                    
                    // Ajax request
                    else if (StrContains(HTTP_req, "json_get")) {
                        DONOTPRINT = true;
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/html"));
                        client.println(F("Connection: keep-alive"));
                        client.println();
                        
                        // read switch state and send appropriate paragraph text
                        getjSonState(client);
                    }

                    // Interfcace Commands
                    else if (StrContains(HTTP_req, "/?sw_knx=1")) {
                      enableKNX();
                      saveInterfaces();
                    }

                    else if (StrContains(HTTP_req, "/?sw_knx=0")) {
                      disableKNX();
                      saveInterfaces();
                    }

                    else if (StrContains(HTTP_req, "/?sw_rs=1")) {
                      enableRS485();
                      saveInterfaces();
                    }

                    else if (StrContains(HTTP_req, "/?sw_rs=0")) {
                      disableRS485();
                      saveInterfaces();
                    }

                    else if (StrContains(HTTP_req, "/?sw_dsc=1")) {
                      enableDSC();
                      saveInterfaces();
                    }

                    else if (StrContains(HTTP_req, "/?sw_dsc=0")) {
                      disableDSC();
                      saveInterfaces();
                    }
                    
                    // Debug Commands
                    else if (StrContains(HTTP_req, "/?db_main=1")) {
                      debug.MAIN = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_main=0")) {
                      debug.MAIN = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_eth=1")) {
                      debug.ETH = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_eth=0")) {
                      debug.ETH = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_mqtt=1")) {
                      debug.MQTT = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_mqtt=0")) {
                      debug.MQTT = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_knx=1")) {
                      debug.KNX = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_knx=0")) {
                      debug.KNX = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_rs=1")) {
                      debug.RS485 = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_rs=0")) {
                      debug.RS485 = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_dsc=1")) {
                      debug.DSC = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_dsc=0")) {
                      debug.DSC = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_soladin=1")) {
                      debug.SOLADIN = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_soladin=0")) {
                      debug.SOLADIN = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_onewire=1")) {
                      debug.ONEWIRE = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_onewire=0")) {
                      debug.ONEWIRE = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_power=1")) {
                      debug.POWER = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_power=0")) {
                      debug.POWER = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_plant=1")) {
                      debug.PLANTNODE = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_plant=0")) {
                      debug.PLANTNODE = false;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_lcl_sn=1")) {
                      debug.LOCALSENSORS = true;
                      saveDebug();
                    }

                    else if (StrContains(HTTP_req, "/?db_lcl_sn=0")) {
                      debug.LOCALSENSORS = false;
                      saveDebug();
                    }
                    
                    
                    // File on SD Card
                    if ( webFile ) {
                        // Read file and put in TCP buffer.
                        size_t ix;
                        while (webFile.available()) {
                            // Put data in buffer
                            ix = webFile.read(buf, TCP_BUFFER_SIZE);
                            if (!(ix % TCP_BUFFER_SIZE)) {
                              client.write(buf, TCP_BUFFER_SIZE);
                            }
                        }
                        // Flush the final bytes in the buffer
                        client.write(buf, ix);
                        webFile.close();
                    }
                    
                    // display received HTTP request on serial port
                    if( debug.ETH && DONOTPRINT != true ) { Serial.println(HTTP_req); }
                    
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

