#include <ClickButton.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define NEOPIN         0
#define NUMPIXELS      1
Adafruit_NeoPixel LEDS = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

SoftwareSerial slave(4, 5); // RX, TX

uint8_t led3R = 255;
uint8_t led3G = 255;
uint8_t led3B = 255;

char data[5];
bool button[8];

bool F1state = false;

ClickButton btnEncoder(button[7], HIGH);
ClickButton btnMode(button[2], HIGH);
//ClickButton btnOrf(btnAllOff, LOW, CLICKBTN_PULLUP);
ClickButton btnF1(button[0], HIGH);
ClickButton btnF2(button[1], HIGH);
ClickButton btnF3(button[6], HIGH);
ClickButton btnF4(button[3], HIGH);
ClickButton btnF5(button[4], HIGH);

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void setLED(uint8_t LED, uint8_t r, uint8_t g, uint8_t b) {
    LEDS.setPixelColor(LED, r, g, b);
    LEDS.show();
}

//--------------------------------------------------------------------------------------------
// Function to handle button presses, including presets in various modes
//--------------------------------------------------------------------------------------------
void buttonFunc(char* buttonName) {
    if (buttonName == "rotarySingle") {
      if (currentMode == knxMode) {
        F1state = !F1state;
        sendKNX("0/0/9", F1state);
      } else if (currentMode == audioMode) {
        sendAudioState(!PLAYSTATE);
      }
    }
    
    else if (buttonName == "rotaryDouble") {
      SETTINGS = !SETTINGS;
    }

    else if (buttonName == "modeSingle") {
      storedMode++;
      if(storedMode > modeCount) {
        storedMode = 1;
      }
    }
    else if (buttonName == "modeDouble") {
      storedMode--;
      if(storedMode < 1) {
        storedMode = modeCount;
      }
    }
    
    else if (buttonName == "F1Single") {
      if (currentMode == audioMode) {
        sendButton(1);
      }
    }
    else if (buttonName == "F2Single") {
      if (currentMode == audioMode) {
        sendButton(2);
      }
    }
    else if (buttonName == "F3Single") {
      if (currentMode == audioMode) {
        sendButton(3);
      }
    }
    else if (buttonName == "F4Single") {
      if (currentMode == audioMode) {
        sendButton(4);
      }
    }
    else if (buttonName == "F5Single") {
      if (currentMode == audioMode) {
        sendButton(5);
      }
    }
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void buttonTask() {

    if (slave.available() > 0) {
      slave.readBytesUntil((char) 0x0D, data, slave.available());
    }

    if (data[0] == 0x21) {
      for (uint8_t i = 0; i < 8; i++){
        button[i] = bitRead(data[1], i);
      }
    } else if (data[0] == 0x22) {
      Serial.println(data[1], HEX);
      switch (data[1]){
        case 0x01:
        //Serial.println(data[1], HEX);
          if(currentMode == audioMode && VOLUME < 100) {
            VOLUME++;
            sendVolume(VOLUME);
          }
          break;
        case 0x02:
        //Serial.println(data[1], HEX);
          if(currentMode == audioMode && VOLUME > 0) {
            VOLUME--;
            sendVolume(VOLUME);
          }
          break;
      }
    }
    
    // Encoder Button
    btnEncoder.Update(button[7]);
    if (btnEncoder.clicks == 1) buttonFunc ("rotarySingle");  // Press rotary 
    if (btnEncoder.clicks == 2) buttonFunc ("rotaryDouble");  // Double press rotary

    // Mode Button
    btnMode.Update(button[2]);
    if (btnMode.clicks == 1) buttonFunc ("modeSingle");       // Press Mode
    if (btnMode.clicks == 2) buttonFunc ("modeDouble");       // Double press Mode

    // Funtionc Buttons
    btnF1.Update(button[0]);
    if (btnF1.clicks == 1) buttonFunc ("F1Single");           // Press F1
    
    btnF2.Update(button[1]);
    if (btnF2.clicks == 1) buttonFunc ("F2Single");           // Press F2
    
    btnF3.Update(button[6]);
    if (btnF3.clicks == 1) buttonFunc ("F3Single");           // Press F3
    
    btnF4.Update(button[3]);
    if (btnF4.clicks == 1) buttonFunc ("F4Single");           // Press F4
    
    btnF5.Update(button[4]);
    if (btnF5.clicks == 1) buttonFunc ("F5Single");           // Press F5

}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void ledTask() {
  if (currentMode == knxMode) {
    setLED(0, 0, 0, 255);
  } else if (currentMode == audioMode) {
    setLED(0, 0, 255, 0);
  } else if (currentMode == tempMode) {
    setLED(0, 255, 0, 0);
  }
  
}

//--------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------
void setupButtons() {

    slave.begin(57600);

    setLED(0, 0, 0, 255);

    btnEncoder.debounceTime    = 20;
    btnEncoder.multiclickTime  = 200;
    btnEncoder.longClickTime   = 1000;

    btnMode.debounceTime       = 20;
    btnMode.multiclickTime     = 200;
    btnF1.debounceTime         = 20;
    btnF2.debounceTime         = 20;
    btnF3.debounceTime         = 20;
    btnF4.debounceTime         = 20;
    btnF5.debounceTime         = 20;

}
