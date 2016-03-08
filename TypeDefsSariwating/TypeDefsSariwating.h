//--------------------------------------------------------------------------------------------
// Data structures for transfering data between units on JeeNode and RS485
//--------------------------------------------------------------------------------------------
typedef struct {
  uint16_t  	PVvolt;
  uint16_t  	PVamp;
  uint16_t  	ACgridPower;
  uint16_t  	ACgridFreq;
  uint16_t  	ACgridVolt;
  uint16_t  	DeviceTemp;
  uint16_t  	AlarmFlag;
  uint16_t  	TotalPower;
  uint16_t  	TotalOperationTime;   
} payloadSoladin;

typedef struct {
  uint16_t  	Temp;
  uint16_t  	Humidity;
  uint16_t  	freeRam;
  unsigned long uptimeDay;
  uint8_t   	uptimeHour;
  uint8_t   	uptimeMinute;
  uint8_t   	uptimeSecond;
  unsigned long	looptime;
} payloadSoladinNode;

typedef struct {
  byte      	address0;
  byte      	address1;
  byte      	address2;
  byte      	address3;
  byte      	address4;
  byte      	address5;
  byte      	address6;
  byte      	address7;
  uint16_t  	temperature;          
}payloadOneWire;

typedef struct {
  uint16_t  	Temp;
  uint16_t  	freeRam;
  unsigned long uptimeDay;
  uint8_t   	uptimeHour;
  uint8_t   	uptimeMinute;
  uint8_t   	uptimeSecond;
  unsigned long	looptime;
} payloadOneWireNode;

typedef struct {
  uint16_t  	importing;
  uint16_t  	power1;
  uint16_t  	power2;
  uint16_t  	power3;
  uint16_t  	voltage;       
}payloadEmonTX;

typedef struct {
  uint16_t 		Temperature;
  uint16_t 		Humidity;
  uint16_t 		SoilSensor;
  uint16_t 		CO;
  uint16_t 		freeRam;
  unsigned long uptimeDay;
  uint8_t  		uptimeHour;
  uint8_t  		uptimeMinute;
  uint8_t  		uptimeSecond;
  unsigned long	looptime;
} PayloadPlantNode;
