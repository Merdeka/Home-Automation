//GLCD fonts - part of GLCD lib
#include "utility/font_helvB14.h" 	//big numberical digits 
#include "utility/font_clR6x8.h" 	//title
#include "utility/font_clR4x6.h" 	//kwh

double cval, cval2, cval3, cval4;   //values to calulate smoothing

void draw_main_screen()
{
  glcd.clear();
  glcd.fillRect(0,0,128,64,0);
  
  glcd.drawLine(64, 0, 64, 64, WHITE);      //top vertical line
  glcd.drawLine(0, 32, 128, 32, WHITE);     //middle horizontal line 
                
  char str[50];    			 //variable to store conversion 
  glcd.setFont(font_clR6x8);             
  glcd.drawString_P(12,0,PSTR("Using"));
  glcd.drawString_P(69,0,PSTR("Solar PV"));
  glcd.drawString_P(85,33,PSTR("Room"));
   
  if (importing==1) glcd.drawString_P(2,34,PSTR("Importing")); else glcd.drawString_P(2,34,PSTR("Exporting"));
  
  glcd.setFont(font_helvB14);  		//big bold font
                 
  cval = cval + (consuming - cval)*0.50;
  itoa((int)cval,str,10);
  strcat(str,"w");
  glcd.drawString(3,9,str); //ammount of power currently being used 
               
  cval2 = cval2 + (gen - cval2)*0.5;
  itoa((int)cval2,str,10);
  strcat(str,"w");
  glcd.drawString(71,9,str); //pv
               
  cval3 = cval3 + (grid - cval3)*0.5;
  itoa((int)cval3,str,10);
  strcat(str,"w");
  glcd.drawString(3,42,str); //importing / exporting
               
  dtostrf(temp,0,1,str); 
  strcat(str,"c");
  glcd.drawString(74,42,str);   
               
  glcd.setFont(font_clR4x6); //small font - Kwh
  dtostrf((wh_consuming/1000),0,1,str);
  strcat(str,"Kwh today");
  glcd.drawString(6,26,str);
               
  dtostrf((wh_gen/1000),0,1,str);
  strcat(str,"Kwh today");
  glcd.drawString(71,26,str);
                             
  itoa((int)mintemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(68,58,PSTR("Min"));
  glcd.drawString(82,58,str);
               
  itoa((int)maxtemp,str,10); 
  strcat(str,"C");
  glcd.drawString_P(97,58,PSTR("Max"));
  glcd.drawString(111,58,str);

  if (((millis()-last_emontx)<20000) && ((millis()-last_baseNode)<20000)) {
    
  DateTime now = RTC.now();
  glcd.drawString_P(5,58,PSTR("Time:"));
  char str2[5];
  itoa((int)now.hour(),str,10);
  if  (now.minute()<10) strcat(str,":0"); else strcat(str,":"); 
  itoa((int)now.minute(),str2,10);
  strcat(str,str2); 
  glcd.drawString(28,58,str); 
  }
  else
    if ((millis()-last_emontx)>20000) glcd.drawString_P(2,58,PSTR("--emonTx fail--"));
//      else glcd.drawString_P(1,58,PSTR("-BaseNode fail-"));

  glcd.refresh();
                    
}

void backlight_control()
{                                                

  //--------------------------------------------------------------------
  // Turn off backlight and indicator LED's between 11pm and 6am
  //-------------------------------------------------------------------- 
  DateTime now = RTC.now();
  if (now.hour()>0) int hour = now.hour();                    //get hour digit in 24hr from software RTC
  
//  if ((hour > 22) ||  (hour < 5) && ((millis()-last_baseNode)<20000)){       // turn off backlight between 11pm and 6am
//    night=1; 
//    glcd.backLight(0);
//  } else {
//    night=0; 
//    glcd.backLight(220);  
//  }
}
