//************************ Uptime Code - Makes a count of the total up time since last start ****************//
//It will work for any main loop's, that loop moret han twice a second: not good for long delays etc

void uptime(){
  //** Checks For a Second Change *****//  
  if( millis()%1000 <= 500 && uptimeOnce == 0 ){
    uptimeSecondStamp = 1;
    uptimeOnce = 1;
  }
  
  //** Makes Sure Second Count doesnt happen more than once a Second **//
  if( millis()%1000 > 500 ){
    uptimeOnce=0;
  }
 
 if( uptimeSecondStamp == 1 )
 {
   uptimeSecond++;
   uptimeSecondStamp=0;
    
   if( uptimeSecond == 60 )
   {
    uptimeMinute++;
    uptimeSecond=0;
                  
    if( uptimeMinute == 60 )
    {
      uptimeMinute=0;
      uptimeHour++;
                              
       if( uptimeHour == 24 )
       {
        uptimeHour=0;
        uptimeDay++;
       }
     }
  }
 };                      
};

String getUptime() {

  String  uptime =  (String) uptimeDay;
          uptime += "d ";
          uptime += (String) uptimeHour;
          uptime += "h ";
          uptime += (String) uptimeMinute;
          uptime += "m ";
          uptime += (String) uptimeSecond;
          uptime += "s";

  return uptime;
}

// SRam check
uint16_t freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

// Time the Loop in microsecond
void loopTimer() {
  static unsigned long thisMicros = 0;
  static unsigned long lastMicros = 0;

  lastMicros = thisMicros;
  thisMicros = micros();

  plantNode.looptime = thisMicros - lastMicros; // looptime in microseconds | ms = / 1000
}
