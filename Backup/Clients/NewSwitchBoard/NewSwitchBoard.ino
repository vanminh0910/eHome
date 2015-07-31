#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_ID 3
#define DEVICE_TYPE device_switchboard
#define UPDATE_INTERVAL 30000
#define SEND_REPEAT 3

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const byte buttonPin[3] = {5,6,7};
const byte relayPin[3]  = {2,3,4};
const byte debounceDelay = 50; // Debounce time;
byte currentButtonStatus[3]; // Current status of pins 5,6,7
byte lastButtonStatus[3]; // Old status of pins 5,6,7
byte relayStatus[3] = {1, 1, 1}; // Status of relay pins 2,3,4
unsigned long lastDebounceTime = 0; //The last time the output pin was toggled
boolean buttonChangeFlag = false;
uint64_t lastCheckTime;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = 2;
  for(int i=0; i<sizeof(relayStatus); i++) {
    msg.data[i] = relayStatus[i];
  }

  printf("\nMsg size: %d", sizeof(struct message));
  printf("\nMsg Type type: %d", msg.msgType);
  printf("\nDevice ID: %d", msg.deviceID);
  printf("\nDevice type: %d", msg.deviceType);

  // Stop listening so we can talk
  radio.stopListening();
  for (int i=0; i<SEND_REPEAT; i++) {
    bool ok = radio.write( &msg, sizeof(struct message));
    if (ok)
      printf("send update ok... \n");
    else
      printf("send update failed.\n\r");
    delay(50);
  }
      
  // Continue listening
  radio.startListening();
}

void setup()
{
  //Init message
  // Start serial connection
  Serial.begin(57600);;
  
  // Configure input pins and output pins
  for(int i=0; i<sizeof(relayPin); i++)
  {
    pinMode(relayPin[i], OUTPUT); // Configure pin2,3,4 as an output 
    pinMode(buttonPin[i], INPUT_PULLUP); // Configure pin5,6,7 as an pull-up input
    relayStatus[i] = 1; // status of relay is OFF
    digitalWrite(relayPin[i], HIGH); //init OFF all relay
    currentButtonStatus[i] = digitalRead(buttonPin[i]); 
    lastButtonStatus[i] = currentButtonStatus[i];
    //Print out the value of the relay status
    Serial.print("Relay ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(relayStatus[i]); 
  }
  //
  // Print preamble
  //
  
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
  
  // Waiting for Gateway implement the register command
  delay(50); 

  // Initial Health Check Cycle
  lastCheckTime = millis();
//  sendUpdate();
}


void loop()
{

  delay(500);
}

