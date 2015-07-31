#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_TYPE device_soilsensor
#define DEVICE_ID 52
#define SEND_REPEAT 2

const unsigned long UPDATE_INTERVAL = 14400000;
const unsigned long WATERING_DURATION = 30000;

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

int sensorPin = A2;    // select the input pin for the moisture sensor
int buttonPin = 6;      // select the pin for button to manually trigger the pump
int relayPin = 4;      // select the pin for the relay

int threshold = 600;

const int numReadings = 3;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
uint16_t average = 0;                // the average

unsigned long lastCheckTime = 0;
unsigned long wateringStartTime = 0;
boolean isWatering = false;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = MSG_Update;
  msg.data[0] = isWatering;
  msg.data[1] = average;
  msg.data[2] = -1; // not used
  msg.data[3] = -1; // not used
  
  printf("\nMsg size: %d", sizeof(struct message));
  printf("\nMsg Type type: %d", msg.msgType);
  printf("\nDevice ID: %d", msg.deviceID);
  printf("\nDevice type: %d", msg.deviceType);

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
  // record last update time
  lastCheckTime = millis();
}
void setup()
{
  //
  // Print preamble
  //
  Serial.begin(57600);
  printf_begin();
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
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // init OFF all relay
  // initialize all the readings to 0: 
  for (int thisReading=0; thisReading<numReadings; thisReading++)
    readings[thisReading] = 0;
  // Waiting for Gateway implement the register command
  delay(50);
  sendUpdate();
}
void loop()
{
  // turn off watering after amount of time
  if (isWatering) {
    unsigned long lapsedTime = (unsigned long)(millis() - wateringStartTime);
    //Serial.print("Millis: ");Serial.println(millis());
    //Serial.print("Start at: ");Serial.println(wateringStartTime);
    //Serial.print("Lapsed: ");Serial.println(lapsedTime);
    if (lapsedTime > WATERING_DURATION) {
      printf("\nTurn off watering now.");
      digitalWrite(relayPin, LOW);
      isWatering = false;
      sendUpdate();
    }
  } else { // if not watering, check if time to get moisture info
    unsigned long lapsedTime = (unsigned long)(millis() - lastCheckTime);
    if(lapsedTime > UPDATE_INTERVAL ) {
      // reading 3 times and get the average value
      total = 0;
      for (int i=0; i<numReadings; i++) {
        // read from the sensor:  
        readings[i] = analogRead(sensorPin);
        printf("\nSensor value read: %d", readings[i]);
        // add the reading to the total:
        total = total + readings[i];
        delay(50);
      }
      // calculate the average:
      average = total / numReadings;
      printf("\nSensor average value now is: %d", average);
      
      if (average > threshold) {
        printf("\nExceed threshold. Now activate watering");
        digitalWrite(relayPin, HIGH);
        isWatering = true;
        // recording time when watering starts
        wateringStartTime = millis();
      }
      sendUpdate();
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
      } else if (msg.msgType == MSG_ChangeStatus){ 
        if (msg.data[0] == 0 || msg.data[0] == 1) {
          printf("\nData received is valid");
          digitalWrite(relayPin, msg.data[0]);
          isWatering = msg.data[0];
          if (isWatering)
            wateringStartTime = millis();
          sendUpdate();
        } else {
          // invalid data received
          printf("\nData received is invalid");
        }
      } else if (msg.deviceID == BROADCAST_ID){
        if (msg.msgType == MSG_RequireUpdate){
          // Waiting for other clients send status update to Gateway done
          delay(DEVICE_ID*20);
          sendUpdate();
        }
      }
    }
  }
  delay(100);
}
