#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "Adafruit_HTU21DF.h"
#//include <Adafruit_INA219.h>

Adafruit_BMP085  bmp;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

//Adafruit_INA219 ina219(0x44);

void setupI2c() {
  
  Wire.begin();

  // Setup the DS1307 Real Time Clock
  if (! RTC.isrunning()) {
    if( debug.MAIN ) { Serial.println(F("RTC is NOT running!")); }
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // Setup BMP085
  if (!bmp.begin()) {
    if( debug.MAIN ) {
      Serial.println(F("Could not find a BMP085 sensor"));
      while (1);
    }
  }
  if( debug.MAIN ) { Serial.println(F("BMP085 started..")); }

  // Setup HTU21D-F
  if (!htu.begin()) {
    if( debug.MAIN ) { 
      Serial.println(F("Couldn't find a HTU21D-F sensor!"));
      while (1);
    }
  }
  if( debug.MAIN ) { Serial.println(F("HTU21D-F started...")); } 

  // Setup INA219B
/*
  ina219.begin();

  if( debug.MAIN ) { Serial.println(F("INA219B started...")); }
*/
}

void readSensors() {
  sensors.temperature = htu.readTemperature() * 100;
  sensors.humidity = htu.readHumidity();
  sensors.barometer = bmp.readPressure();
//  sensors.voltage = ina219.getBusVoltage_V();
//  sensors.current = ina219.getCurrent_mA();
}

