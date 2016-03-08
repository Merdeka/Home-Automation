#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "Adafruit_HTU21DF.h"

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

//--------------------------------------------------------------------------------------------
// Initialize TSL256 Light sensor
//--------------------------------------------------------------------------------------------
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(0x39, 12345);

//--------------------------------------------------------------------------------------------
//  Configures the gain and integration time for the TSL2561
//--------------------------------------------------------------------------------------------
void configureTSL2561(void) {
  
  // You can also manually set the gain or enable auto-gain support
  // tsl.setGain(TSL2561_GAIN_1X);      // No gain ... use in bright light to avoid sensor saturation
  // tsl.setGain(TSL2561_GAIN_16X);     // 16x gain ... use in low light to boost sensitivity
  tsl.enableAutoRange(true);            // Auto-gain ... switches automatically between 1x and 16x
  
  // Changing the integration time gives you better sensor resolution (402ms = 16-bit data)
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      // fast but low resolution
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  // medium resolution and speed
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  // 16-bit data but slowest conversions
  
}
//--------------------------------------------------------------------------------------------
//  Get TSL2561 Light Sensor and store in global
//--------------------------------------------------------------------------------------------
void getTSL2561() {

  // Get a new sensor event 
  sensors_event_t event;
  tsl.getEvent(&event);
  
  sensorData.lightSensor = event.light;
}

void setupSensors() {
  
  // Setup HTU21D-F Lightsensor
  if (!htu.begin()) {
    #ifdef DEBUG
      Serial.println(F("Couldn't find a HTU21D-F sensor!"));
      while (1);
    #endif
  }
  #ifdef DEBUG
     Serial.println(F("HTU21D-F started..."));
  #endif
  
}

