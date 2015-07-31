#include <SPI.h>
#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_TYPE device_pirswitch
#define DEVICE_ID 21
#define UPDATE_INTERVAL 300000
#define SEND_REPEAT 2

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const byte relayPin = 2;
const byte pirPin = 7;
const byte ldrPin = A5;
const int ldrThreshold = 100;
const int pirDelay = 15000;

unsigned long lastCheckTime = 0;
boolean switchOn = false;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = MSG_Update;
  msg.data[0] = switchOn;
  msg.data[1] = analogRead(ldrPin);
  msg.data[2] = digitalRead(pirPin);
  msg.data[3] = -1;
  
  printf("\nMsg size: %d", sizeof(struct message));
  printf("\nMsg Type type: %d", msg.msgType);
  printf("\nDevice ID: %d", msg.deviceID);
  printf("\nDevice type: %d", msg.deviceType);
  printf("\nData 1: %d", msg.data[0]);
  printf("\nData 2: %d", msg.data[1]);
  printf("\nData 3: %d", msg.data[2]);
  printf("\nData 4: %d", msg.data[3]);
  printf("\n\n");

  // Stop listening so we can talk
  radio.stopListening();
  for(int i=0; i<SEND_REPEAT; i++) {
    bool ok = radio.write( &msg, sizeof(struct message));
    if (ok)
      printf("\nSend update ok...");
    else
      printf("\nSend update failed.");
    delay(50);
  }
      
  // Continue listening
  radio.startListening();
}
void setup()
{
  //
  // Print preamble
  //
  Serial.begin(57600);
  printf_begin();

  // use pin 5 as vcc pin for ldr
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // init OFF all relay
  switchOn = false; // status of relay is OFF
  
  //
  // Setup and configure rf radio
  //	
  	
  radio.begin();

  // Optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);

  // Optionally, reduce the payload size.  seems to
  // Improve reliability
  radio.setPayloadSize( sizeof(struct message) );
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(0);

  //
  // Open pipes to other nodes for communication
  //
  
  radio.openWritingPipe(GATEWAY_PIPE);
  radio.openReadingPipe(1, CLIENT_PIPE+DEVICE_ID);
 
  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  
  // Waiting for Gateway implement the register command
  delay(50); 
  
  sendUpdate();
 
  // Initial Health Check Cycle
  lastCheckTime = millis();
}
void loop()
{
  // turn off light if no motion for long time
  if (switchOn) {
    unsigned long lapsedTime = (unsigned long)(millis() - lastCheckTime);
    if (lapsedTime > pirDelay) {
      printf("\nNo motion for long time. Turn off light now.");
      digitalWrite(relayPin, LOW);
      switchOn = false;
      sendUpdate();
    }
  }
  
  // check if motion detected
  if (digitalRead(pirPin) == HIGH)
  {
    printf("\nMotion detected");
    lastCheckTime = millis();

    // read the env light status
    // (which goes from 0 - 1023)    
    int ldrValue = analogRead(ldrPin);
    
    printf("\nLdr value: %d", ldrValue);
    
    if (ldrValue < ldrThreshold) {
      if (!switchOn) {
        printf("\nNow is night and motion detected so turn on the light");
        digitalWrite(relayPin, HIGH);
        switchOn = true;
        sendUpdate();
      }
    }
  }

  if (radio.available())
  {
    printf("\nRF24 data received");
    struct message msg;
    bool done = false;
    // Dump the payloads until we've gotten everything
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &msg, sizeof(struct message));
    }
    printf("\n\n\n");
    printf("\nMessage: msgType - %d;", msg.msgType);
    printf("\nDeviceID - %d", msg.deviceID);
    printf("\nDeviceType - %d;", msg.deviceType);
    printf("\nData 1: %d", msg.data[0]);
    printf("\nData 2: %d", msg.data[1]);
    printf("\nData 3: %d", msg.data[2]);
    printf("\nData 4: %d", msg.data[3]);
    printf("\n\n\n");
    // Check destination ID
    if (msg.deviceID == DEVICE_ID){
      // Update status if Switchboard received command request status from Gateway
      if (msg.msgType == MSG_RequireUpdate){
        sendUpdate();
        // Update Health Check Time
        lastCheckTime = millis();
      } else if (msg.msgType == MSG_ChangeStatus){ 
        if (msg.data[0] != 0 && msg.data[0] != 1) {
          printf("\nData received is valid");
          digitalWrite(relayPin, msg.data[0]);
          switchOn = msg.data[0];
          sendUpdate();
          // Update Health Check Time
          lastCheckTime = millis();
        } else {
          // invalid data received
          printf("\nData received is invalid");
        }
      } else if (msg.deviceID == BROADCAST_ID){
        if (msg.msgType == MSG_RequireUpdate){
          // Waiting for other clients send status update to Gateway done
          delay(DEVICE_ID*20);
          sendUpdate();
          // Update Health Check Time
          lastCheckTime = millis();
        }
      }
    }
  }
  delay(50);
}
