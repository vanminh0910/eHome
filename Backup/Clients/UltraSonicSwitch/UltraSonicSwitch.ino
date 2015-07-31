#include <NewPing.h>
#include <SPI.h>
#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_ID 31
#define DEVICE_TYPE 30
#define UPDATE_INTERVAL 300000
#define MAX_DELAY 200

#define TRIGGER_PIN  6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     7  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 150 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const byte setCsdSampleButton = 8;
const byte setPirDelayButton = 2;
const byte relayPin  = 5;
const byte csdPin = A3;
const int scdSampleValueDefault = 20;                 // default
const int pirDelayDefault = 30;                      // (s) default

unsigned long lastCheckTime = 0;
//unsigned long checkTime = 0;
int pirDelay = 0;
int scdSampleValue = 0;
int csdValue = 0;
byte relayStatus = 0;
boolean isSwitchOn = false;
boolean isNight = false;

int currentDistance = 0;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = 2;
  msg.data[0] = relayStatus;
  msg.data[1] = scdSampleValue;
  msg.data[2] = pirDelay;
  
  printf("\nMsg size: %d", sizeof(struct message));
  printf("\nMsg Type type: %d", msg.msgType);
  printf("\nDevice ID: %d", msg.deviceID);
  printf("\nDevice type: %d", msg.deviceType);

  // Stop listening so we can talk
  radio.stopListening();
  bool ok = radio.write( &msg, sizeof(struct message));
  if (ok)
    printf("send update ok... \n");
  else
    printf("send update failed.\n\r");
      
  // Continue listening
  radio.startListening();
}
void setup()
{
  // Init message
  // Start serial connection
  Serial.begin(9600);
  
  //
  // Print preamble
  //
  
  printf_begin();
  pinMode(csdPin, INPUT);
  pinMode(setCsdSampleButton, INPUT_PULLUP);
  pinMode(setPirDelayButton, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT); // Configure pin2,3,4 as an output 
  digitalWrite(relayPin, LOW); // init OFF all relay
  relayStatus = 0; // status of relay is OFF
    
  scdSampleValue = EEPROM.read(0);
  pirDelay = EEPROM.read(1);
 
  if (scdSampleValue == 0 || scdSampleValue >= 255){
    scdSampleValue = scdSampleValueDefault;
    EEPROM.write(0, scdSampleValue);
  }
  
  if (pirDelay == 0 || pirDelay >= 255)
  {
    pirDelay = pirDelayDefault;
    EEPROM.write(1, pirDelay);
  }
  
  //
  // Setup and configure rf radio
  //	
  	
  radio.begin();

  // Optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);

  // Optionally, reduce the payload size.  seems to
  // Improve reliability
  radio.setPayloadSize( sizeof(struct message) );

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
  
  // test
  printf("\nscdSampleValue = %d", scdSampleValue);
  printf("\npirDelay = %d", pirDelay);
  printf("\neeprom csd = %d", EEPROM.read(0));
  printf("\neeprom pirDelay = %d", EEPROM.read(1));
  printf("**********************************************");

}
void loop()
{ 
  if (!digitalRead(setCsdSampleButton))    // take sample light intensity
  {
    printf("\nOld csd sample value = %d", scdSampleValue);
    while(!digitalRead(setCsdSampleButton))
    {
      ;// waitting for set csd sample done
    }
    scdSampleValue = analogRead(csdPin)/4;
    EEPROM.write(0, scdSampleValue);
    printf("\nnew scdSampleValue value = %d", EEPROM.read(0));
    sendUpdate();
  }
  if (!digitalRead(setPirDelayButton))
  {
    printf("\nold pirDelay = %d", pirDelay);
    unsigned long checkTime = millis();
    while(!digitalRead(setPirDelayButton))
    {
      ;// waitting for set pir delay done
    }
    pirDelay = 10*(fabs(millis() - checkTime))/1000;

    if (pirDelay > MAX_DELAY)
    {
      pirDelay = MAX_DELAY;
    }
    EEPROM.write(1, pirDelay);
    printf("\nnew pirDelay = %d", EEPROM.read(1));
    sendUpdate();
  
  }    
  // read the input on analog csdPin:
  // (which goes from 0 - 1023)
  
  csdValue = analogRead(csdPin)/4;
  
  // check presense by ultrasonic sensor
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  int newDistance = uS / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.print("New distance: ");
  Serial.print(newDistance);
  Serial.println("cm");
  
  if (csdValue < scdSampleValue)    // is night
  {
    printf("\nNow is night");
    isNight = true;
    if (newDistance<(currentDistance-5) || newDistance>(currentDistance+5))
    {
      printf("\nMotion detected");
      currentDistance = newDistance;
      lastCheckTime = millis();
      if (!isSwitchOn)
      {
        digitalWrite(relayPin, HIGH);
        relayStatus = 1;
        isSwitchOn = true;
        sendUpdate();
        lastCheckTime = millis();
      }
    } 
    
    if (isSwitchOn && ((millis() - lastCheckTime)/1000 > pirDelay))
    {
      printf("\nTurn off light due to no activity");
      digitalWrite(relayPin, LOW);
      relayStatus = 0;
     
      isNight = false;
      isSwitchOn = false;
      sendUpdate();
      lastCheckTime = millis();
    } 
  } 
  else 
  {
    if (isNight && (newDistance<(currentDistance-5) || newDistance>(currentDistance+5)))
    {
      lastCheckTime = millis();
    } 
    if (isSwitchOn && ((millis() - lastCheckTime)/1000 > pirDelay))
    {
      digitalWrite(relayPin, LOW);
      relayStatus = 0;
      isNight = false;
      isSwitchOn = false;
      sendUpdate();
      lastCheckTime = millis();
    }   
  }
  if (radio.available())
  {
    printf("\nData comes");
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
    printf("\ndeviceID - %d", msg.deviceID);
    printf("\ndeviceType - %d;", msg.deviceType);
    printf("\ndata1: %d", msg.data[0]);
    printf("\ndata2: %d", msg.data[1]);
    printf("\ndata3: %d", msg.data[2]);
    printf("\n\n\n");
    // Check destination ID
    if (msg.deviceID == DEVICE_ID){
      // Update status if Switchboard received command request status from Gateway
      if (msg.msgType == RequireUpdate){
        sendUpdate();
        // Update Health Check Time
        lastCheckTime = millis();
      } else if (msg.msgType == ChangeStatus){ 
        if (msg.data[0] == 0 || msg.data[0] == 1) {
          printf("\nRelay data received is valid");
          digitalWrite(relayPin, msg.data[0]);
          relayStatus = msg.data[0];
        }        
        sendUpdate();
        // Update Health Check Time
        lastCheckTime = millis();
      } else if (msg.msgType == SendSettings) {
        // get cds value
        if (msg.data[0] > 0 && msg.data[0] < 255) {
          scdSampleValue = msg.data[0];
          EEPROM.write(0, scdSampleValue);
          printf("\nnew scdSampleValue value = %d", EEPROM.read(0));
        }
        // get pirDelay value
        if (msg.data[1] > 0 && msg.data[1] <= MAX_DELAY) {
          pirDelay = msg.data[1];
          EEPROM.write(1, pirDelay);
          printf("\nnew pirDelay = %d", EEPROM.read(1));
        }
        sendUpdate();
      } else if (msg.deviceID == BROADCAST_ID){
        if (msg.msgType == RequireUpdate){
          // Waiting for other clients send status update to Gateway done
          delay(DEVICE_ID*20);
          sendUpdate();
          // Update Health Check Time
          lastCheckTime = millis();
        }
      }
    }
  }
 
  // Health Check Cycle
  if(lastCheckTime > millis())                                // Timer of Arduino resets after 50 days
  {
    lastCheckTime = 0;
  }
  else if((millis() - lastCheckTime) > UPDATE_INTERVAL )      // Send Update Status after 1 minutes
  {
    sendUpdate();
    // Update Health Check Time
    lastCheckTime = millis();
  }
  delay(50);
}
