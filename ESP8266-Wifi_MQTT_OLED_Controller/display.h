#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ESP8266_SSD1322.h>
#include "icons.h"
#include <Fonts/FreeSansBold9pt7b.h>

//ESP8266 Pins
#define OLED_CS     15  // Pin 19, CS - Chip select
#define OLED_DC     2   // Pin 20 - DC digital signal
#define OLED_RESET  16  // Pin 15 -RESET digital signal

//hardware SPI - only way to go. Can get 110 FPS
ESP8266_SSD1322 display(OLED_DC, OLED_RESET, OLED_CS);

// Define draw locations
#define icon_x_coord 4
#define icon_y_coord 11

#define alloff_x_coord 234
#define alloff_y_coord 9

#define major_text_x_coord 40
#define major_text_y_coord 23
#define minor_text_x_coord 42
#define minor_text_y_coord 29

#define f1_text_centre_x_coord 24
#define f2_text_centre_x_coord 76
#define f3_text_centre_x_coord 128
#define f4_text_centre_x_coord 180
#define f5_text_centre_x_coord 232
#define fb_text_y_coord 56

int16_t x = major_text_x_coord + 10;
int16_t minX;

String audioMinor(String text) {

  String temp;
  if(text.length() > 27) {
    temp = text.substring(0,26);
    temp += "..";
  } else {
    temp = text;
  }
  return temp;
}

//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
void drawOLED() {

      if (currentMode == settingsMode) {
        display.clearDisplay();
        display.setTextWrap(false);
      
        display.fastDrawBitmap(1, 14, espBitmap, 128, 36, WHITE);
    
        display.setFont();
        display.setCursor(10, 53);
        display.print(F("Connect Everything"));
    
        display.drawLine(132, 0, 132, 63, WHITE);
    
        display.setCursor(136, 5);
        String wifiTxt = F("Wifi: ");
          wifiTxt += wifiStatus;
        display.print(wifiTxt);
    
        display.setCursor(136, 15);
        String ipTxt = F("IP: ");
          ipTxt += displayIP(WiFi.localIP());
        display.print(ipTxt);
    
        display.setCursor(136, 25);
        String mqttTxt = F("MQTT: ");
          mqttTxt += mqttStatus;
        display.print(mqttTxt);
  
        display.setCursor(136, 35);
        String uptimeTxt = F("Up: ");
          uptimeTxt += getUptime();
        display.print(uptimeTxt);
    
        display.setCursor(138, 56);
        display.print(F("(C)2016 Sariwating"));
        
        display.display();
    }


    /*          Draw KNX Mode           */
    if (currentMode == knxMode) {
  
      display.clearDisplay();
  
      // Draw icon
      display.fastDrawBitmap(icon_x_coord, icon_y_coord, light_bits, icon_width, icon_height, WHITE);

      // Draw F1-F5
      String f1Text = "Keuken";
      String f2Text = "Kolom";
      String f3Text = "Raam";
      String f4Text = "Badkamr";
      String f5Text = "Bnkstel";
      
      display.setFont();
      display.setCursor(f1_text_centre_x_coord - ((f1Text.length() / 2)*6), fb_text_y_coord);
      display.print(f1Text);

      display.setCursor(f2_text_centre_x_coord - ((f2Text.length() / 2)*6), fb_text_y_coord);
      display.print(f2Text);

      display.setCursor(f3_text_centre_x_coord - ((f3Text.length() / 2)*6), fb_text_y_coord);
      display.print(f3Text);

      display.setCursor(f4_text_centre_x_coord - ((f4Text.length() / 2)*6), fb_text_y_coord);
      display.print(f4Text);
      
      display.setCursor(f5_text_centre_x_coord - ((f5Text.length() / 2)*6), fb_text_y_coord);
      display.print(f5Text);
  
      display.setFont();
      display.setCursor(2, 0);
      display.print(F("HOUSE"));
  
      // Write date and time
      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(major_text_x_coord, major_text_y_coord);
      display.print(getDateTime());
  
      //
      display.setFont();
      display.setCursor(minor_text_x_coord, minor_text_y_coord);
      display.print(F("Slaapkamer"));
  
      // Draw "All Off"
      display.setFont();
      display.setCursor(234, 0);
      display.print(F("OFF"));
      
      display.fastDrawBitmap(alloff_x_coord, alloff_y_coord, alloff_bits, alloff_width, alloff_height, WHITE);
  
      display.display();
    }

    /*          Draw Audio Mode            */
    if (currentMode == audioMode) {
  
      display.clearDisplay();

      if (PLAYSTATE == true) {
        
        // 
        display.setFont(&FreeSansBold9pt7b);
        if(TITLE.length() > 16) {
          minX = -8 * TITLE.length();      
          display.setCursor(x, 20);
          display.print(TITLE);
        
          if(--x < minX) x = 215; //major_text_x_coord + 10;
          
          display.fillRect(0, 0, major_text_x_coord + 10, 64, BLACK);
          display.fillRect(215, 0, 215, 64, BLACK);
        } else {
          display.setCursor(major_text_x_coord + 10, major_text_y_coord);
          display.print(TITLE);
        }  
    
        // Draw Artist
        display.setFont();
        display.setCursor(minor_text_x_coord + 10, minor_text_y_coord);
        display.print(audioMinor(ARTIST));
        
        if(SONGSTATUSBAR) {
          // Draw Song Position
          display.setFont();
          display.setCursor(minor_text_x_coord + 10, minor_text_y_coord + 12);
          display.print(SONGTIME);
 
          display.setCursor(197, minor_text_y_coord + 12);
          display.print(SONGDURATION);
  
          display.drawRect(minor_text_x_coord + 42, minor_text_y_coord + 15, 100, 3, WHITE);
          display.fillRect(minor_text_x_coord + 42, minor_text_y_coord + 15, SONGPERCENTAGE, 3, WHITE);
        }

        // Play Icon
        display.fastDrawBitmap(major_text_x_coord - 4, 14, play_bits, 8, 8, WHITE);
        
      } else {
        
        // 
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(major_text_x_coord + 10, major_text_y_coord);
        display.print(F("Paused"));
    
        //
        display.setFont();
        display.setCursor(minor_text_x_coord + 10, minor_text_y_coord);
        display.print(getDateTime());

        // Pause Icon
        display.fastDrawBitmap(major_text_x_coord - 4, 14, pause_bits, 8, 8, WHITE);
      }

      // Draw F1-F5
      String f1Text = "NPO 2";
      String f2Text = "NPO 3";
      String f3Text = "538";
      String f4Text = "BBC Xtr";
      String f5Text = "Rgaeton";
      
      display.setFont();
      display.setCursor(f1_text_centre_x_coord - ((f1Text.length() / 2)*6), fb_text_y_coord);
      display.print(f1Text);

      display.setCursor(f2_text_centre_x_coord - ((f2Text.length() / 2)*6), fb_text_y_coord);
      display.print(f2Text);

      display.setCursor(f3_text_centre_x_coord - ((f3Text.length() / 2)*6), fb_text_y_coord);
      display.print(f3Text);

      display.setCursor(f4_text_centre_x_coord - ((f4Text.length() / 2)*6), fb_text_y_coord);
      display.print(f4Text);
      
      display.setCursor(f5_text_centre_x_coord - ((f5Text.length() / 2)*6), fb_text_y_coord);
      display.print(f5Text);
  
      // Draw icon
      display.fastDrawBitmap(icon_x_coord, icon_y_coord, audio_bits, icon_width, icon_height, WHITE);
  
      display.setFont();
      display.setCursor(2, 0);
      display.print(F("AUDIO"));

      // Draw "Volume
      display.setFont();
      display.setCursor(237, 0);
      display.print(printDigits(VOLUME));
      
      display.fastDrawBitmap(alloff_x_coord, alloff_y_coord, volume_bits, alloff_width, alloff_height, WHITE);
  
      display.display();
    }

     /*          Draw Temperature Mode           */
    if (currentMode == tempMode) {
  
      display.clearDisplay();
  
      // Draw icon
      display.fastDrawBitmap(icon_x_coord, icon_y_coord, temperature_bits, icon_width, icon_height, WHITE);
  
      display.setFont();
      display.setCursor(5, 0);
      display.print(F("TEMP"));
  
      // 
      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(major_text_x_coord, major_text_y_coord);
      String temp = INSIDETEMPERATURE;
      temp += F(" C");
      display.print(temp);

      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(major_text_x_coord + 105, major_text_y_coord);
      temp = OUTSIDETEMPERATURE;
      temp += F(" C");
      display.print(temp);
  
      //
      display.setFont();
      display.setCursor(minor_text_x_coord, minor_text_y_coord);
      display.print(F("Woonkamer"));

      display.setFont();
      display.setCursor(minor_text_x_coord + 105, minor_text_y_coord);
      display.print(F("Outside"));

      display.drawLine(128, 11, 128, 37, WHITE);
  
      // Draw "All Off"
      display.setFont();
      display.setCursor(234, 0);
      display.print(F("OFF"));
      
      display.fastDrawBitmap(alloff_x_coord, alloff_y_coord, alloff_bits, alloff_width, alloff_height, WHITE);

      display.display();
    }
    
}



