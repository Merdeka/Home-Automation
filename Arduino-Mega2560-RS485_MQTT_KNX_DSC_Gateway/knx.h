#include <KnxDevice.h>

// Definition of the Communication Objects attached to the device
KnxComObject KnxDevice::_comObjectsList[] =
{
  /* Index 0 = ALL OFF               */    KnxComObject(G_ADDR(15,0,0), KNX_DPT_1_001  /* 1.001 B1 DPT_Switch */ , COM_OBJ_SENSOR /* Sensor Output */ ) , 
  /* Index 1 = TIME                  */    KnxComObject(G_ADDR(14,0,1), KNX_DPT_10_001 /* KNX_DPT_10_001      */ , COM_OBJ_SENSOR /* Time Output */) , 
  /* Index 2 = DATE                  */    KnxComObject(G_ADDR(14,0,2), KNX_DPT_11_001 /* KNX_DPT_11_001      */ , COM_OBJ_SENSOR /* Date Output */) ,
  /* Index 3 = WOONKAMER TEMPERATURE */    KnxComObject(G_ADDR(2,0,0),  KNX_DPT_9_001  /* 2 byte float DPT    */ , COM_OBJ_SENSOR /* Sensor Output */ ) ,
  /* Index 4 = AFZUIGKAP VERLICHTING */    KnxComObject(G_ADDR(0,0,14), KNX_DPT_1_001  /* 1.001 B1 DPT_Switch */ , 0x2F /* Logical Input Object with Init Read */) ,
};

const byte KnxDevice::_comObjectsNb = sizeof(_comObjectsList) / sizeof(KnxComObject); // do no change this code

void sendKNXTime() {
  // KNX Time N3N5r2N6r2N6
  // NNN NNNNN | rr NNNNNN | rr NNNNNN
  // Hours     | Minutes   | Seconds
  
  DateTime now = RTC.now();
  utc = now.unixtime();
  localTZ = myTZ.toLocal(utc, &tcr);

  byte knxTime[3];
  knxTime[0] = hour(localTZ)   & B00011111;
  knxTime[1] = minute(localTZ) & B00111111;
  knxTime[2] = second(localTZ) & B00111111;

  Knx.write(1, knxTime);
}


void sendKNXDate() {
  // KNX Date r3N5r4N4r1U7
  // rrr NNNNN | rrrr NNNN | r UUUUUUU
  // Day       | Month     | Year
  
  DateTime now = RTC.now();
  utc = now.unixtime();
  localTZ = myTZ.toLocal(utc, &tcr);

  byte knxDate[3];
  knxDate[0] =  day(localTZ)          & B00011111;
  knxDate[1] =  month(localTZ)        & B00001111;
  knxDate[2] = (year(localTZ) - 2000) & B01111111;

  Knx.write(2, knxDate);
}

void setupKNX() {
    
  if (Knx.begin(Serial1, P_ADDR(1,1,10)) == KNX_DEVICE_ERROR) {
    if( debug.MAIN ) { Serial.println(F("KNX init ERROR, stop here!!")); }
    while(1);
  }
  if( debug.MAIN ) { Serial.println(F("KNX started..")); }
}

void stopKNX() {

  Knx.end();
  Serial1.end();
}

void enableKNX() {
  if( debug.MAIN ) { Serial.println(F("KNX Protocol Enabled")); }
  interfaces.KNX = true;
  setupKNX();
}

void disableKNX() {
  if( debug.MAIN ) {  Serial.println(F("KNX Protocol Disabled")); }
  interfaces.KNX = false;
  stopKNX();
}

// Callback function to handle com objects updates
void knxEvents(byte index) {

  DynamicJsonBuffer  knxBuffer;
  JsonObject& knxJson = knxBuffer.createObject();
  
  switch (index)
  {
    case 1 : // 14/0/1 TIME
      sendKNXTime();
      break;
    case 2 : // 14/0/2 DATE
      sendKNXDate();
      break;
    case 4 : // 0/0/14 LED KEUKEN
      // we arrive here when object index 1 has been updated              
      knxJson[F("Address")] = "0/0/14";
      knxJson[F("Name")]    = "Keuken LED";
      knxJson[F("Value")]   = Knx.read(4);

      if( debug.KNX ) { knxJson.prettyPrintTo(Serial); }

      sendMQTT("iot/sariwating/knx", knxJson);
      break;
      
    default:
      // code to treat remaining objects updates
      break;
  }
}
