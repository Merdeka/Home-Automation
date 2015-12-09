
// New version of the Room Node, derived from rooms.pde
// 2010-10-19 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

// see http://jeelabs.org/2010/10/20/new-roomnode-code/
// and http://jeelabs.org/2010/10/21/reporting-motion/

// The complexity in the code below comes from the fact that newly detected PIR
// motion needs to be reported as soon as possible, but only once, while all the
// other sensor values are being collected and averaged in a more regular cycle.

// Circuit:
 
/*       Port 2---* (Digital 5)
                  |
                  \
                  /
                  \ R1 4K7
                  /
                  |
                  |
 analog Port 2----* (Analog 1)
                  |
                  |
                  *----> nail 1
                 
                  *----> nail 2
                  |
                  \
                  /
                  \ R2 220
                  /
                  |
                  |
        Port 3 ---* (Digital 6)
 */

#define DEBUG

#include <JeeLib.h>
#include <avr/sleep.h>
#include <util/atomic.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define LDR_PORT    4   // defined if LDR is connected to a port's AIO pin

#define SOIL_PORT    3 // defined if SOIL is connected to a port's AIO & DIO pin

// Data wire is plugged into port 4 on the Arduino
#define ONE_WIRE_BUS 4

//--------------------------------------------------------------------------------------------
// DS18B20 temperature setup - onboard sensor 
//--------------------------------------------------------------------------------------------
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature. 
double temp;

#define MEASURE_PERIOD  600 // how often to measure, in tenths of seconds
#define RETRY_PERIOD    10  // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5   // maximum number of times to retry
#define ACK_TIME        10  // number of milliseconds to wait for an ack
#define REPORT_EVERY    5   // report every N measurement cycles

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

// The scheduler makes it easy to perform various tasks at various times:

enum { MEASURE, REPORT, TASK_END };

static word schedbuf[TASK_END];
Scheduler scheduler (schedbuf, TASK_END);

// Other variables used in various places in the code:

static byte reportCount;    // count up until next report, i.e. packet send
static byte myNodeID;       // node ID used for this unit

// This defines the structure of the packets which get sent out by wireless:

struct {
    byte NodeType;  // NodeType &H14 = PlantNode
    byte light;     // light sensor: 0..255
    int soil;       // soil sensor:
    int temp;       // temperature: -500..+500 (tenths)
    byte lobat :1;  // supply voltage dropped under 3.1V: 0..1
} payload;

// Conditional code, depending on which sensors are connected and how:
    Port ldr (LDR_PORT);
    Port soil (SOIL_PORT);
    
    OneWire ds18s20 (4); // 1-wire temperature sensors, uses DIO port 4

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

// wait a few milliseconds for proper ACK to me, return true if indeed received
static byte waitForAck() {
    MilliTimer ackTimer;
    while (!ackTimer.poll(ACK_TIME)) {
        if (rf12_recvDone() && rf12_crc == 0 &&
                // see http://talk.jeelabs.net/topic/811#post-4712
                rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | myNodeID))
            return 1;
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
    }
    return 0;
}

// readout all the sensors and other values
static void doMeasure() {
  
       payload.NodeType = 0x14;
    
       ldr.digiWrite2(1);  // enable AIO pull-up
       byte light = ~ ldr.anaRead() >> 2;
       ldr.digiWrite2(0);  // disable pull-up to reduce current draw
       payload.light = light;
       
       // drive a current through the divider in one direction
       soil.digiWrite(1);

       // wait a moment for capacitance effects to settle
       delay(1250);
       
       payload.soil = ( soil.anaRead() );
       
       soil.digiWrite(0);

       // Get temperatue from onboard sensor
       sensors.requestTemperatures();
       temp = sensors.getTempCByIndex(0);
       if (temp > 60) doMeasure();
       payload.temp = (int) ( temp * 100);
       oneWire.depower();
       
       payload.lobat = rf12_lowbat();
       

}

// periodic report, i.e. send out a packet and optionally report on serial port
static void doReport() {
    rf12_sleep(RF12_WAKEUP);
    while (!rf12_canSend())
        rf12_recvDone();
    rf12_sendStart(0, &payload, sizeof payload, RADIO_SYNC_MODE);
    rf12_sleep(RF12_SLEEP);

    #ifdef DEBUG
        Serial.print("NodeType: ");
        Serial.println((int) payload.NodeType);
        Serial.print("Light: ");
        Serial.println((int) payload.light);
        Serial.print("Soil: ");
        Serial.println((int) payload.soil);
        Serial.print("Temp: ");
        Serial.println((int) payload.temp);
        Serial.print("Bat: ");
        Serial.println((int) payload.lobat);
    #endif
}

static void serialFlush () {
    #if ARDUINO >= 100
        Serial.flush();
    #endif  
    delay(2); // make sure tx buf is empty before going back to sleep
}

void setup () {
    #ifdef DEBUG
        Serial.begin(57600);
        Serial.print("\n[plantNode]");
        myNodeID = rf12_config();
        serialFlush();
    #else
        myNodeID = rf12_config(0); // don't report info on the serial port
    #endif
    
    rf12_sleep(RF12_SLEEP); // power down

    reportCount = REPORT_EVERY;     // report right away for easy debugging
    scheduler.timer(MEASURE, 0);    // start the measurement loop going
}

void loop () {
    #ifdef DEBUG
        Serial.print('.');
        serialFlush();
    #endif
    
    switch (scheduler.pollWaiting()) {

        case MEASURE:
            // reschedule these measurements periodically
            scheduler.timer(MEASURE, MEASURE_PERIOD);
    
            doMeasure();

            // every so often, a report needs to be sent out
            if (++reportCount >= REPORT_EVERY) {
                reportCount = 0;
                scheduler.timer(REPORT, 0);
            }
            break;
            
        case REPORT:
            doReport();
            break;
    }
}
