#include "RTClib.h"
#include <TimeLib.h>
#include <Timezone.h>

RTC_DS1307 RTC;
unsigned long inline ntpUnixTime (UDP &udp);

//Central European Time (Amsterdam, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone myTZ(CEST, CET);

//If TimeChangeRules are already stored in EEPROM, comment out the three
//lines above and uncomment the line below.
//Timezone myTZ(100);       //assumes rules stored at EEPROM address 100

TimeChangeRule *tcr;        //pointer to the time change rule, use to get TZ abbrev
time_t utc, localTZ;

byte ntpServer[] = { 193, 79, 237, 14 };    // ntp1.nl.net NTP server

String getTimeStamp() {

  DateTime now = RTC.now();
  utc = now.unixtime();
  localTZ = myTZ.toLocal(utc, &tcr);
  
  char timeStamp[] = "00/00/0000 00:00:00";
  sprintf(timeStamp,"%02d/%02d/%04d %02d:%02d:%02d", day(localTZ), month(localTZ), year(localTZ), hour(localTZ), minute(localTZ), second(localTZ));

  return (String) timeStamp;
};

void getNTP() {
  
  unsigned long unixTime = ntpUnixTime(udp);

  // Set RTC Clock
  DateTime ntp = DateTime(unixTime);
  if( ntp.year() >= 2100 ) {
    if( debug.MAIN ) { Serial.println(F("NTP update invalid..")); }
  } else {
    RTC.adjust(DateTime(unixTime));

    if( interfaces.KNX ) {
      sendKNXTime();
      sendKNXDate();
    }
    
    if( debug.MAIN ) { 
      Serial.print(F("RTC updated from NTP: "));
      Serial.println(getTimeStamp());
    }
  }
}

unsigned long inline ntpUnixTime( UDP &udp ) {
  static int udpInited = udp.begin(123); // open socket on arbitrary port

  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  // Fail if WiFiUdp.begin() could not init a socket
  if (! udpInited)
    return 0;

  // Clear received data from possible stray received packets
  udp.flush();

  // Send an NTP request
  if (! (udp.beginPacket(ntpServer, 123) // 123 is the NTP port
   && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
   && udp.endPacket()))
    return 0;       // sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;   // poll every this many ms
  const byte maxPoll = 15;    // poll up to this many times
  int pktLen;       // received packet length
  for (byte i=0; i<maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0;       // no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv/8);

  // Discard the rest of the packet
  udp.flush();
  return time - 2208988800ul;   // convert NTP time to Unix time
}
