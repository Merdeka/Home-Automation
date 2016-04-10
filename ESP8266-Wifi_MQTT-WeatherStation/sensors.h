#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS2423.h>

#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 11

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire ONEWIRE(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&ONEWIRE);
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

// define the 1-Wire address of the DS2423 counter here
uint8_t DS2423_address[] = { 0x1D, 0x80, 0x90, 0x01, 0x00, 0x00, 0x00, 0x11 };

DS2423 ds2423(&ONEWIRE, DS2423_address);

//Variables for wind speed
long wsTime;
int timeDifference;
int WindCounter1, WindCounter2, RevsPerSecx100 = 0;
#define METER_PER_SECOND 1.096;
#define KMS_PER_HOUR 3.9477;

uint8_t windDirectionOffset = 3;

//General read variables
int HighByte, LowByte;

//Variables for the AD conversion and wind direction
uint8_t WindPos, WindDirection, ChA, ChB, ChC, ChD;

uint8_t numberOfDevices; // Number of temperature devices found

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
int GetWindDirection() {
  
  ONEWIRE.reset();
  ONEWIRE.skip();                // skip ROM, no parasite power on at the end
  ONEWIRE.write(0x3c, 0);        // convert
  ONEWIRE.write(0x0f, 0);        // all channels
  ONEWIRE.write(0xaa, 0);        // preset to all zeros

  ONEWIRE.read();                //Read back 16 bit CRC
  ONEWIRE.read();                // which we're not interested in in this code!

  while(1)                  // wait for conversion to complete
  {
    if(ONEWIRE.read() == 0xff)
    {
      break;
    }
  }

  delay(1);
  ONEWIRE.reset();
  ONEWIRE.skip();                // skip ROM, no parasite power on at the end
  ONEWIRE.write(0xaa, 0);        // read memory
  ONEWIRE.write(0x00, 0);        // channel A
  ONEWIRE.write(0x00, 0);        // locations 0000 and 0001

  //Channel A
  LowByte = ONEWIRE.read();      //Get the returned low byte (always 0 if running at 8 bit resolution)
  HighByte = ONEWIRE.read();     //Get the high byte (the 8 bit value of the input)
  ChA = HighByte/50;        //Divide by 50 to get volts (rounded badly to volts only)

  //Channel B
  LowByte = ONEWIRE.read();      //Get the returned low byte (always 0 if running at 8 bit resolution)
  HighByte = ONEWIRE.read();     //Get the high byte (the 8 bit value of the input)
  ChB = HighByte/50;        //Divide by 50 to get volts (rounded badly to volts only)

  //Channel C
  LowByte = ONEWIRE.read();      //Get the returned low byte (always 0 if running at 8 bit resolution)
  HighByte = ONEWIRE.read();     //Get the high byte (the 8 bit value of the input)
  ChC = HighByte/50;        //Divide by 50 to get volts (rounded badly to volts only)

  //Channel D
  LowByte = ONEWIRE.read();      //Get the returned low byte (always 0 if running at 8 bit resolution)
  HighByte = ONEWIRE.read();     //Get the high byte (the 8 bit value of the input)
  ChD = HighByte/50;        //Divide by 50 to get volts (rounded badly to volts only)

  WindDirection = 0;
  //      A         B         C         D  POS.
  if(ChA>=4 && ChB>=4 && ChC==2 && ChD>=4){WindDirection=1;}
  if(ChA>=4 && ChB==3 && ChC==3 && ChD>=4){WindDirection=2;}
  if(ChA>=4 && ChB==2 && ChC>=4 && ChD>=4){WindDirection=3;}
  if(ChA==3 && ChB==3 && ChC>=4 && ChD>=4){WindDirection=4;}
  if(ChA==2 && ChB>=4 && ChC>=4 && ChD>=4){WindDirection=5;}
  if(ChA==2 && ChB>=4 && ChC>=4 && ChD==0){WindDirection=6;}
  if(ChA>=4 && ChB>=4 && ChC>=4 && ChD==0){WindDirection=7;}
  if(ChA>=4 && ChB>=4 && ChC==0 && ChD==0){WindDirection=8;}
  if(ChA>=4 && ChB>=4 && ChC==0 && ChD>=4){WindDirection=9;}
  if(ChA>=4 && ChB==0 && ChC==0 && ChD>=4){WindDirection=10;}
  if(ChA>=4 && ChB==0 && ChC>=4 && ChD>=4){WindDirection=11;}
  if(ChA==0 && ChB==0 && ChC>=4 && ChD>=4){WindDirection=12;}
  if(ChA==0 && ChB>=4 && ChC>=4 && ChD>=4){WindDirection=13;}
  if(ChA==0 && ChB>=4 && ChC>=4 && ChD==2){WindDirection=14;}
  if(ChA>=4 && ChB>=4 && ChC>=4 && ChD==2){WindDirection=15;}
  if(ChA>=4 && ChB>=4 && ChC==3 && ChD==2){WindDirection=16;}

  //Add the wind direction offset here and figure out if it's more than 17, if it istake 17 away from it
  WindDirection = WindDirection + windDirectionOffset;
  if(WindDirection > 16)
  {
    WindDirection = WindDirection - 16;
  }

  return WindDirection;
}

//--------------------------------------------------------------------------------------------
// Set up the DS2450 in the weather station
//--------------------------------------------------------------------------------------------
void Configure_2450()
{

  // Shouldn't be needed if using parasite power but seems to be! Need to look into further
  ONEWIRE.reset();
  ONEWIRE.write(0xcc, 0);        // skip ROM, no parasite power on at the end
  ONEWIRE.write(0x55, 0);        // Write memory
  ONEWIRE.write(0x1c, 0);        // write to 001c
  ONEWIRE.write(0x00, 0);        // Vcc operation
  ONEWIRE.write(0x40, 0);        // "
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits
  //

  ONEWIRE.reset();
  ONEWIRE.skip();                // skip ROM, no parasite power on at the end
  ONEWIRE.write(0x55,0);         // Write memory
  ONEWIRE.write(0x08,0);         // write beginning at 0008 (Channel A Control/Status)
  ONEWIRE.write(0x00,0);         // 

  ONEWIRE.write(0x08,0);         // 0008 (Channel A Control/Status) - 8 bits
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x01, 0);        // 0009 (Channel A Control/Status) - 5.1 VDC range
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x08,0);         // 000A (Channel B Control/Status) - 8 bits
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x01, 0);        // 000B (Channel B Control/Status) - 5.1 VDC range
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x08,0);         // 000C (Channel C Control/Status) - 8 bits
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x01, 0);        // 000D (Channel C Control/Status) - 5.1 VDC range
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x08,0);         // 000E (Channel D Control/Status) - 8 bits
  ONEWIRE.read();                // Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits

  ONEWIRE.write(0x01, 0);        // 000F (Channel D Control/Status) - 5.1 VDC range
  ONEWIRE.read();                //Read back 16 bit CRC
  ONEWIRE.read();                //
  ONEWIRE.read();                // Read back verification bits
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Returns the revolutions per second x 100 to allow for decimal places to be worked out
/////////////////////////////////////////////////////////////////////////////////////////////
int CalcRevolutionsPerSecondx100(int WindCounter1, int WindCounter2, int WindDelayMilliSeconds)
{
  if(WindCounter2 < WindCounter1)         //If the counter has gone past 0...
  {
    WindCounter2 = WindCounter2 + 255;    // Add 255 for this comparision (it'll sort itself out for next time)
  }
  //We must /2  in the next formula as there are 2 counts per revolution.
  //Multiplying by 100 so I can pass back an int and then work the decimal places out in the loop.
  RevsPerSecx100 = (((WindCounter2 - WindCounter1) * 100) / 2) / (WindDelayMilliSeconds / 1000);
  return(RevsPerSecx100);
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void setupSensors() {
  
  // Start up the library
  sensors.begin();
  ds2423.begin(DS2423_COUNTER_A|DS2423_COUNTER_B);
  Configure_2450();
  
  WindCounter1 = ds2423.getCount(DS2423_COUNTER_A);    // Set the wind counter variables...
  WindCounter2 = WindCounter1;                         // to the same amount and...
  wsTime = millis();                                   // store the current millis.
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  Serial.print(numberOfDevices);
  Serial.println(F(" Devices."));
  
  for(uint8_t i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      if (tempDeviceAddress[0] == 0x10) {
        // set the resolution to TEMPERATURE_PRECISION bit (For Temperature Sensor)
        sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
      }
    }
  }
  
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void sensorTask() {

  ds2423.update();
  
  for(uint8_t i = 0; i < numberOfDevices; i++) {
      // Search the wire for address
      if(sensors.getAddress(tempDeviceAddress, i)) {
          switch (tempDeviceAddress[0]) {
            case 0x10://  DS18S20 Temperature sensor
              sensors.requestTemperatures();
              sensorData.temperature = (sensors.getTempC(tempDeviceAddress)) * 100;
              break;
            case 0x1d://  DS2423 4k ram with counter
              if (ds2423.isError()) {
                  Serial.println(F("Error reading counter"));
              } else {                 
                  WindCounter2 = ds2423.getCount(DS2423_COUNTER_A);   // Get the current counter value
                  timeDifference = millis() - wsTime;                 // Work out the time since the last count
                  wsTime = millis();                                  // Reset the time count
                  RevsPerSecx100 = CalcRevolutionsPerSecondx100(WindCounter1, WindCounter2, timeDifference);
                  WindCounter1 = ds2423.getCount(DS2423_COUNTER_A);   // Take the counter to compare next time
                  sensorData.windspeed_mps = (RevsPerSecx100) * METER_PER_SECOND
                  sensorData.windspeed_kph = (RevsPerSecx100) * KMS_PER_HOUR
                  
              }
              break;
            case 0x20://  DS2450  Quad A/D converter
              //Example of windDriection Offset: If the weather station is facing
              //East is 4 points away from North so the offset is 4.
              sensorData.winddirection = GetWindDirection();
              break;
          }
      }
  }
}
