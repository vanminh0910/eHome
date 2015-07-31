#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
                      //writeup here: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
#include <NewPing.h>
#include "Common.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        22    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************
#define ONBOARD_LED    9  // Moteinos have LEDs on D9
#define EXT_LED    5  // External led light
#define ARMED_ON_LED   A0  // external LED when armed mode is on
#define BUZZER_PIN  3  // external LED when alarm is activated

#define PIR_PIN 4
#define LDR_PIN A3
#define BUTTON_PIN 6    // the pushbutton pin

// Ultrasonic PIN config
#define TRIGGER_PIN  8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     7  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 150 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    115200
#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(0);}
#define DEBUGln(input) {Serial.println(input); delay(0);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif

// Button debounce data
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned long lastMotionDetectedTime = 0;  // the last time motion is detected
unsigned long lightOnDelay = 10000;    // how long should we turn on flash light
 
// RFM69 configuration
RFM69 radio;
boolean promiscuousMode = false; //set to 'true' to sniff all packets on the same network
Payload data;

volatile boolean motionDetected = false;

boolean armedMode = false; 
boolean lightOn = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  char buff[50];
  sprintf(buff, "\neGuard node starting. ");
  DEBUGln(buff);
  //radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  //radio.encrypt(ENCRYPTKEY);  
  sprintf(buff, "\nWorking at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(EXT_LED, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(ARMED_ON_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  //attachInterrupt(1, motionIRQ, RISING);
  //Blink(ONBOARD_LED, 500); 
  Blink(BUZZER_PIN, 200); 
  delay(2000);
  DEBUGln("Setup done");
}

void loop() {  
  motionDetected = (digitalRead(PIR_PIN)==HIGH);
  DEBUG("Motion detected: ");
  DEBUGln(motionDetected);
  // check if armed mode is on
  if (armedMode)
  {
    digitalWrite(ARMED_ON_LED, HIGH);
    /*
    // check if ultrasonic sensor gets obstacle
    unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
    float distance = uS / US_ROUNDTRIP_CM;
    DEBUG("Ping: ");
    DEBUG(distance); // Convert ping time to distance in cm and print result (0 = outside set distance range)
    DEBUGln("cm");    
    // if yes, check if motion is detected or not
    if (distance != 0 && distance < MAX_DISTANCE) {
      // ping one more time for sure
      delay(50);
      uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
      distance = uS / US_ROUNDTRIP_CM;
      DEBUG("Ping again: ");
      DEBUG(distance); // Convert ping time to distance in cm and print result (0 = outside set distance range)
      DEBUGln("cm");
      if (distance != 0 && distance < MAX_DISTANCE ) {//&& motionDetected) {
        DEBUGln("Alarm mode is triggered");
        //1. Trigger alarm and send message to server
        sendUpdate(-1, -1, 1);
        //2. Activate buzzer
        //3. On off light continuously until reset
        while (true) {
          digitalWrite(EXT_LED, HIGH);
          digitalWrite(BUZZER_PIN, HIGH);
          delay(500);
          digitalWrite(EXT_LED, LOW);
          digitalWrite(BUZZER_PIN, LOW);
          delay(500);
        } // end while
      } // end second ping again
    } // end first ping
    */
  } else {
    digitalWrite(ARMED_ON_LED, LOW);
  }

  /*
  if (motionDetected) { // check if motion is detected?
    //DEBUGln("Motion is detected");
    if (!lightOn) {
      //DEBUGln("Light off, checking light sensor");
      // if flash light is off, check light sensor and turn on if too dark
      DEBUGln(digitalRead(LDR_PIN));
      if (digitalRead(LDR_PIN)) {
        digitalWrite(EXT_LED, HIGH);
        lightOn = true;
        // start timer to turn off light automatically
        lastMotionDetectedTime = millis();
        sendUpdate(1, -1, -1);
        DEBUGln("Too dark, turn on light now");
      }
    } else {
      //DEBUGln("Light on, reset timer");
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
  */
  
  // check button pressed and using debounce detected
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading == LOW) {
    DEBUGln("Button pressed");
  }

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    DEBUGln("Button status changed");
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle armed mode if the new button state is LOW
      if (buttonState == LOW) {
        armedMode = !armedMode;
        DEBUGln("Alarm mode is toggled");
        sendUpdate(-1, 1, -1);
      }
    }
  }
  
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
  
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
  return;
  DEBUGln("Sending alarm to server");

  // Build the data payload and send to gateway
  data.nodeId = NODE_ID;
  data.data[0] = lightOn;
  data.data[1] = alarmOn;
  data.data[2] = alarmActivated;
  data.data[3] = -1;
 
  DEBUG("Sending data to gateway (");
  DEBUG(sizeof(data));
  DEBUG(" bytes) ... ");
  digitalWrite(ONBOARD_LED, HIGH);
  if (radio.send(GATEWAY_ID, (const void*)(&data), sizeof(data)), 0) {
    DEBUG(" Sent ok!");
  } else {
    DEBUG(" Sent failed...");
  }
  digitalWrite(ONBOARD_LED, LOW);
  DEBUGln(); 
}
