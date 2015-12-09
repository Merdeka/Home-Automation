//------------------------------------------------------------------------------------------------------------------------------------------------
// emonGLCD Solar PV monitor example - compatiable with Arduino 1.0 

// emonGLCD documentation http://openEnergyMonitor.org/emon/emonglcd
// solar PV monitor build documentation: http://openenergymonitor.org/emon/applications/solarpv

// For use with emonTx setup with 2CT with CT 1 monitoring consumption/grid and CT 2 monitoring PV generation .
// The CT's should be clipped on with the orientation so grid reading is postive when importing and negative when exporting. Generation reading should always be positive. 

// Correct time is updated via NanodeRF which gets time from internet, this is used to reset Kwh/d counters at midnight. 

// Temperature recorded on the emonglcd is also sent to the NanodeRF for online graphing

// this sketch is currently setup for type 1 solar PV monitoring where CT's monitor generation and consumption separately
// The sketch assumes emonx.power1 is consuming/grid power and emontx.power2 is solarPV generation
// to use this sketch for type 2 solar PV monitoring where CT's monitor consumption and grid import/export using an AC-AC adapter to detect current flow direction 
//    -change line 220-221- see comments in on specific lines. See Solar PV documentation for explination 

// GLCD library by Jean-Claude Wippler: JeeLabs.org
// 2010-05-28 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
//
// Authors: Glyn Hudson and Trystan Lea
// Part of the: openenergymonitor.org project
// Licenced under GNU GPL V3
// http://openenergymonitor.org/emon/license

//-------------------------------------------------------------------------------------------------------------------------------------------------
//#define DEBUG

//JeeLab libraires		       http://github.com/jcw
#include <JeeLib.h>		    // ports and RFM12 - used for RFM12B wireless
#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib

#include <GLCD_ST7565.h>            // Graphical LCD library 
#include <avr/pgmspace.h>           // Part of Arduino libraries - needed for GLCD lib

GLCD_ST7565 glcd;

#define MYNODE 20            //Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     //frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 30            //network group, must be same as emonTx and emonBase

const int emonTx_nodeID = 10;
const int baseNode_nodeID = 15;
const int soladinNode_nodeID = 11;

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------
typedef struct { 
    byte NodeType;
    byte importing :1;
    int power1;
    int power2;
    int power3;
    int Vrms;
} PayloadTX;       // assume that power1 is consuming/grid and power 2 is solar PV generation 
PayloadTX emontx;

typedef struct {
    byte NodeType;  // NodeType 0x16 = Soladin Node
    int PVvolt;
    int PVamp;
    int ACgridPower;
    int ACgridFreq;
    int ACgridVolt;
    int DeviceTemp;
    int AlarmFlag;
    int TotalPower;
    int TotalOperationTime;
    byte lobat :1;  // supply voltage dropped under 3.1V: 0..1
} PayloadSoladin;
PayloadSoladin soladin;

typedef struct {
    byte NodeType;
    int hour;
    int mins;
    int sec;
    int temperature;
    byte BackLight :1;
} PayloadBase;
PayloadBase baseNode;
//---------------------------------------------------

//--------------------------------------------------------------------------------------------
// Power variables
//--------------------------------------------------------------------------------------------
int importing, night;                                  //flag to indicate import/export
double consuming, gen, grid, wh_gen, wh_consuming;     //integer variables to store ammout of power currenty being consumed grid (in/out) +gen
unsigned long whtime;                    	       //used to calculate energy used per day (kWh/d)

//--------------------------------------------------------------------------------------------
// DS18B20 temperature setup - onboard sensor 
//--------------------------------------------------------------------------------------------
double temp,maxtemp,mintemp;

//--------------------------------------------------------------------------------------------
// Software RTC setup
//-------------------------------------------------------------------------------------------- 
RTC_Millis RTC;
int hour;
  
//-------------------------------------------------------------------------------------------- 
// Flow control
//-------------------------------------------------------------------------------------------- 
unsigned long last_emontx;                   // Used to count time from last emontx update
unsigned long last_baseNode;                   // Used to count time from last emontx update
unsigned long slow_update;                   // Used to count time for slow 10s events
unsigned long fast_update;                   // Used to count time for fast 100ms events
  
//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup () {
    rf12_initialize(MYNODE, freq,group);
    
    glcd.begin(0x18);    //begin glcd library and set contrast 0x20 is max, 0x18 seems to look best on emonGLCD
    glcd.backLight(220); //max 255
   
    #ifdef DEBUG 
      Serial.begin(57600);
      print_glcd_setup();
    #endif
  
    temp = 0;     // get inital temperture reading
    mintemp = temp; maxtemp = temp;          // reset min and max
    
    RTC.begin(DateTime(__DATE__, __TIME__)); //sets the RTC to the date & time this sketch was compiled
}
//--------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop () {
  
    //--------------------------------------------------------------------------------------------
    // 1. On RF recieve
    //--------------------------------------------------------------------------------------------  
    if (rf12_recvDone()){
      if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
      {
        int node_id = (rf12_hdr & 0x1F);
        
        if (node_id == emonTx_nodeID)             // === EMONTX node ID ====
        {
          last_emontx = millis();                 // set time of last update to now
          emontx = *(PayloadTX*) rf12_data;       // get emontx payload data
          
          delay(100);                             // delay to make sure printing finished
          power_calculations();                   // do the power calculations
        }
        
        if (node_id == soladinNode_nodeID)             // === Soladin node ID ====
        {
          soladin = *(PayloadSoladin*) rf12_data;       // get Soladin payload data
          
          delay(100);                             // delay to make sure printing finished
          power_calculations();                   // do the power calculations
        }
        
        if (node_id == baseNode_nodeID)           // ====baseNode  node ID ====
        {
          last_baseNode = millis(); 
          baseNode = *(PayloadBase*) rf12_data;   // get baseNode payload data
          
          RTC.adjust(DateTime(2012, 1, 1, baseNode.hour, baseNode.mins, baseNode.sec));  // adjust emonglcd software real time clock
          
            int iLCD = (int)baseNode.BackLight;
              if (iLCD == 0){
                glcd.backLight(220);
              }else {
                glcd.backLight(220);
              }
              
          
          delay(100);                             // delay to make sure printing and clock setting finished
                                      
        }
        else
        {
          Serial.print("Unknown Node: ");
          Serial.println(node_id);
        }
      }
    }
    
    if (millis()<50000){   //reset min and max at the beginning of the sketch 
      mintemp=0;
      maxtemp=0;}
      
   if ((millis()>50000) && (millis() < 60000))   
     mintemp=temp;
     
    //--------------------------------------------------------------------
    // Things to do every 10s
    //--------------------------------------------------------------------
    if ((millis()-slow_update)>10000)
    {
       slow_update = millis();
       
       // Get temperatue
       temp = (float)baseNode.temperature / 100;
       if (temp > maxtemp) maxtemp = temp;
       if (temp < mintemp) mintemp = temp;
       
 }
   
    //--------------------------------------------------------------------
    // Update the display every 200ms
    //--------------------------------------------------------------------
    if ((millis()-fast_update)>200)
    {
      fast_update = millis();

      backlight_control();
      draw_main_screen();
       // Control led's
    
    }

   
} //end loop
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------
// Calculate power and energy variables
//--------------------------------------------------------------------
void power_calculations()
{
  DateTime now = RTC.now();
  int last_hour = hour;
  hour = now.hour();
  if (last_hour == 23 && hour == 00) { wh_gen = 0; wh_consuming = 0; }
  
  gen = soladin.ACgridPower;
  grid=emontx.power1;  
  consuming=gen + emontx.power1; 
             
  if (gen > consuming) {
    importing=0; //set importing flag
    grid= grid*-1;	//set grid to be positive - the text 'importing' will change to 'exporting' instead.
  } else importing=1;
            
  //--------------------------------------------------
  // kWh calculation
  //--------------------------------------------------
  unsigned long lwhtime = whtime;
  whtime = millis();
  double whInc = gen * ((whtime-lwhtime)/3600000.0);
  wh_gen=wh_gen+whInc;
  whInc = grid *((whtime-lwhtime)/3600000.0);
  wh_consuming=wh_consuming+whInc;
  //---------------------------------------------------------------------- 
}


