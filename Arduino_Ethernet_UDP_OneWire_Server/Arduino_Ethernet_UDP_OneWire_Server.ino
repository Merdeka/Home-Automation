/*
 DS1820 - reads DS1820 sensors and Broadcasts the readings over UDP

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program;
 if not, see <http://www.gnu.org/licenses/>.
*/


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <OneWire.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 60, 241);
IPAddress dest(192, 168, 60, 10);

unsigned int udpPort = 9988;      // local port to listen on
int  address_index=0;
byte address_storage[64];

#define CONVERT     0x44
#define READSCRATCH 0xBE

// The DS1820 is connected on pin 5
OneWire  ds(5);

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char startupMessage[] = "Sariwating.net OneWire Server\n";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(udpPort);

  Serial.begin(57600);
  
    byte addr[8];
  while(1) {
     if( ! ds.search(addr)) {
       ds.reset_search();
       break;
     }
     else {
       if ( OneWire::crc8( addr, 7) != addr[7] && addr[0] != 0x10)  // CRC Check or is not an DS1820
       {  
         Serial.print("CRC is not valid or device is not an DS1820!\n");
       }
       else {
          Serial.print("Found:");
          for (int i =0; i< 8; i++) {
             address_index++;
             Serial.print(addr[i], HEX);
             Serial.print(":");
             address_storage[address_index]= addr[i];
           }
           Serial.println("");
       }
     }
     delay(10);
  }
  Serial.print("We got ");
  Serial.print((int) (address_index/8) );
  Serial.print(" sensors\n");

    Udp.beginPacket(dest, udpPort);
    Udp.write(startupMessage);
    Udp.endPacket();

}

void getSensorData(char *buffer)
{ 
  int index = 1;
  Serial.println("In function getSensorData()");
  for (int i=1; i <= (address_index/7); i++) {
    byte addr[8];
    Serial.print("Sensor: ");
    Serial.println(i);
    char udp[412]="";
    strcat(buffer, "OneWire Sensor: ");
    strcat(udp, "OneWire Sensor: ");
//    sprintf(buffer, "%s %x",buffer, i);
//    strcat(buffer, ": ");
    for (int e =0; e< 8; e++) {
      addr[e] = address_storage[index];
      index++;
    }
    Serial.println("Nach address_storage -> adresse");
    for (int r=0; r<8; r++) {
        char address[10];
        sprintf(address, "%x", addr[r]);
        strcat(buffer, address);
        strcat(udp, address);
        Serial.print(addr[r],HEX);
    }
    Serial.println("Nach message_data.append(adresse zu hexa; ");
    strcat(buffer, ": ");
    strcat(udp, ": ");
    byte present = 0;
    byte data[12];
    // OK we have sensor in addr we read it out!
    ds.reset();
    ds.select(addr);
    ds.write(CONVERT,1);
    delay(1000);
    //delay(750);
    present = ds.reset();                    // Reset device
    ds.select(addr);                         // Select device
    ds.write(READSCRATCH);                   // Read Scratchpad 
    
    for (int b = 0; b < 9; b++) { // get 9 bytes from sensor
      data[b] = ds.read();
    }
    
    if(OneWire::crc8( data, 8) == data[8])  // Check CRC is valid
    {
      // CRC is ok
      float read_temp=((data[1]<<8) | data[0]) >> 1 ;            // Divide the temperature by 2
      float temp_count=float(data[7] - data[6])/(float)data[7];  // Convert to real temperature
      float real_temp = ((float)read_temp-0.25)+temp_count;
      char buff[12];
      tempToAscii(real_temp, buff);
      Serial.println(buff);
      strcat(buffer, buff);
      strcat(udp, buff);
      
      Udp.beginPacket(dest, udpPort);
      Udp.write(udp);
      Udp.endPacket();
    }
    else {
      Serial.print("Error on CRC8");
      strcat(buffer, "Cound not CRC8 data");
    }
    strcat(buffer, "\n");
   }
}

void tempToAscii(double temp, char *buff)
{
  int frac;
  frac=(unsigned int)(temp*1000)%1000;
  itoa((int)temp,buff,10);
  strcat(buff,".");
  itoa(frac,&buff[strlen(buff)],10); //put the frac after the deciaml
}

void loop() {

    char buffer[412]="";
    getSensorData(buffer);
    delay(15000);
}
