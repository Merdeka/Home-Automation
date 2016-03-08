#include <Esp.h>

uint8_t NUMOFCHIPS = 5;
const long FlashID[5] = {
  0x1640E0,   // 00
  0x1640EF,   // 01
  0x1440C8,   // 02
  0x1340C8,   // 03
  0x1340EF    // 04
}; // Little Endian
const String FlashDesc[5] = {
  "BergMicro BG25Q32: 32M-bit / 4M-byte",   // 00
  "WINBOND W25Q32: 32M-bit / 4M-byte",      // 01
  "GIGADEVICE GD25Q40 4M-bit / 512K-byte",  // 02
  "GIGADEVICE GD25Q80 8M-bit / 1M-byte",    // 03
  "WINBOND W25Q40 4M-bit / 512K-byte"       // 04
};

void storeChipId() {
  sensorData.ChipId = ESP.getChipId();
  sensorData.FlashChipId = ESP.getFlashChipId();
}

String getChipId() {

  String id = "0x";
  id += String(sensorData.ChipId, HEX);
  
  return id;
}

String getFlashChipId() {

  String flash = "0x";
  flash += String(sensorData.FlashChipId, HEX);
  flash.toLowerCase();
  
  for(uint8_t i=0 ; i < NUMOFCHIPS ; i++)
  {
    if (ESP.getFlashChipId() == FlashID[i])
    {
      flash = FlashDesc[i];
    }
  }

  return flash;
}

