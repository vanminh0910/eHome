#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_ID 1
#define DEVICE_TYPE device_switchboard
#define UPDATE_INTERVAL 300000
#define SEND_REPEAT 2

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const byte buttonPin[3] = {6,7,8};
const byte relayPin[3]  = {3,4,5};
const byte debounceDelay = 50; // Debounce time;
byte currentButtonStatus[3]; // Current status of pins 5,6,7
byte lastButtonStatus[3]; // Old status of pins 5,6,7
byte relayStatus[3] = {0, 0, 0}; // Status of relay pins 2,3,4
unsigned long lastDebounceTime = 0; //The last time the output pin was toggled
boolean buttonChangeFlag = false;
uint64_t lastCheckTime;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = MSG_Update;
  msg.data[0] = -1;
  msg.data[1] = -1;
  msg.data[2] = -1;
  msg.data[3] = -1;
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
  
  // Configure input pins and output pins
  for(int i=0; i<sizeof(relayPin); i++)
  {
    pinMode(relayPin[i], OUTPUT); // Configure pin2,3,4 as an output 
    pinMode(buttonPin[i], INPUT_PULLUP); // Configure pin5,6,7 as an pull-up input
    relayStatus[i] = 0; // status of relay is OFF
    digitalWrite(relayPin[i], LOW); //init OFF all relay
    currentButtonStatus[i] = digitalRead(buttonPin[i]); 
    lastButtonStatus[i] = currentButtonStatus[i];
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
  sendUpdate();
}


void loop()
{

  // Read input pins, debounce and check button is clicked or not
  for (byte i=0; i<sizeof(buttonPin); i++)
  {
    byte reading = digitalRead(buttonPin[i]); // Read the state of the switch
    
    // If the switch changed, due to noise or pressing:
    if( reading != lastButtonStatus[i])                   
    {
      lastDebounceTime = millis(); // Reset debouncing timer
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay)      
    {
      // Check button is clicked, change status of relay
      if( reading != currentButtonStatus[i])                 
      { 
        currentButtonStatus[i] = reading;
        relayStatus[i] = 1 - relayStatus[i];
        buttonChangeFlag = true;
        //Print out the value of the relay status
        printf("\nRelay %d has changed status to %d", i, relayStatus[i]);
      }
    }
    lastButtonStatus[i] = reading;
  }
    
  
  if ( radio.available() )
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
            for(int i=0; i<sizeof(relayPin); i++) {
              // only update what is changed
              if (msg.data[i] == 0 || msg.data[i] == 1)
                relayStatus[i] = msg.data[i];
            }
            buttonChangeFlag = true;
            delay(500);
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
  
    // Control relay and send latest status of switch board
    if( buttonChangeFlag )
    {
      for (byte i=0; i<sizeof(relayPin); i++) 
      {
        digitalWrite(relayPin[i], relayStatus[i]);
      }
      
      // After Switchboard implement command, send latest status to Gateway
      printf("\nSend updated status due to button pressed");
      sendUpdate();
      // Update Health Check Time
      lastCheckTime = millis();
      
      // Clear buttonChangeFlag when update status done
      buttonChangeFlag = false;
    }
    
    // Health Check Cycle
    if(lastCheckTime > millis())                       // Timer of Arduino resets after 50 days
    {
      lastCheckTime = 0;
    }
    else if((millis() - lastCheckTime) > UPDATE_INTERVAL )      // Send Update Status after 1 minutes
    {
      sendUpdate();
      // Update Health Check Time
      lastCheckTime = millis();
    }  
}

