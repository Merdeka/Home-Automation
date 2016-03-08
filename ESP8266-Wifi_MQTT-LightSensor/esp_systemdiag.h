// Just Some basic Definitions used for the Up Time Logger
long    uptimeDay = 0;
uint8_t uptimeHour, uptimeMinute, uptimeSecond, uptimeSecondStamp, uptimeOnce = 0;

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
  
  char uptimeTime[] = "00:00:00";
  sprintf(uptimeTime,"%02dh %02dm %02ds", uptimeHour, uptimeMinute, uptimeSecond);

  String uptime = (String) uptimeDay;
  uptime += F("d ");
  uptime += (String) uptimeTime;

  return (String) uptime;
}
