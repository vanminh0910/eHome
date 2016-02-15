#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
                      //writeup here: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
#include "Common.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        22    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************

#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    115200
#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(0);}
#define DEBUGln(input) {Serial.println(input); delay(0);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif

#define ONBOARD_LED    9  // Moteinos have LEDs on D9
#define EXT_LED    5  // External led light

#define PIR_PIN 9
#define LDR_PIN 4

unsigned long lastMotionDetectedTime = 0;  // the last time motion is detected
unsigned long lightOnDelay = 15000;    // how long should we turn on flash light
 
// RFM69 configuration
RFM69 radio;
boolean promiscuousMode = false; //set to 'true' to sniff all packets on the same network
Payload data;

volatile boolean motionDetected = false;
boolean lightOn = true;

void setup() {
  Serial.begin(SERIAL_BAUD);
  char buff[50];
  sprintf(buff, "\neMotionSwitch node starting. ");
  DEBUGln(buff);
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);  
  sprintf(buff, "\nWorking at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  //pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(EXT_LED, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  attachInterrupt(1, motionIRQ, RISING);
  //Blink(ONBOARD_LED, 500); 
  delay(10000);
  DEBUGln("Setup done");
}

void motionIRQ()
{
  motionDetected=true;
}

void loop() {  
  //motionDetected = (digitalRead(PIR_PIN)==HIGH);
  //DEBUG("Motion detected: ");
  //DEBUGln(motionDetected);

  if (motionDetected) { // check if motion is detected?
    DEBUGln("Motion is detected");
    if (!lightOn) {
      //DEBUGln("Light off, checking light sensor");
      //DEBUGln(digitalRead(LDR_PIN));
      // if flash light is off, check light sensor and turn on if too dark      
      if (digitalRead(LDR_PIN)) {
        DEBUGln("Too dark, turn on light now");
        digitalWrite(EXT_LED, HIGH);
        lightOn = true;
        // start timer to turn off light automatically
        lastMotionDetectedTime = millis();
        sendUpdate(1, -1, -1);
      }
    } else {
      //DEBUGln("Light currently on, reset timer");
      // light is on, reset motion detected timer
      lastMotionDetectedTime = millis();        
    }
  } else if (lightOn) {
    //DEBUGln("No motion and light on, check timer");
    // check if light is on for enough time, then turn off
    DEBUGln(millis() - lastMotionDetectedTime);
    if ((millis() - lastMotionDetectedTime) > lightOnDelay) {
      digitalWrite(EXT_LED, LOW);
      lightOn = false;
      sendUpdate(0, -1, -1);
      DEBUGln("Turn off timer");
    }    
  }
  
  motionDetected = false;
  delay(100);
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sendUpdate(int lightOn, int alarmOn, int alarmActivated)
{
  //return;

  // Build the data payload and send to gateway
  data.nodeId = NODE_ID;
  data.data[0] = lightOn;
  data.data[1] = -1;
  data.data[2] = -1;
  data.data[3] = -1;
 
  DEBUG("Sending data to gateway (");
  DEBUG(sizeof(data));
  DEBUG(" bytes) ... ");
  //digitalWrite(ONBOARD_LED, HIGH);
  if (radio.send(GATEWAY_ID, (const void*)(&data), sizeof(data)), 0) {
    DEBUG(" Sent ok!");
  } else {
    DEBUG(" Sent failed...");
  }
  //digitalWrite(ONBOARD_LED, LOW);
  DEBUGln(); 
}
