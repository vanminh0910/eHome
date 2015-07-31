#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_ID 3
#define DEVICE_TYPE 12
#define UPDATE_INTERVAL 300000

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const byte buttonPin[2] = {6,7};
const byte relayPin[2]  = {3,4};
const byte debounceDelay = 50; // Debounce time;
byte currentButtonStatus[2]; // Current status of pins 5,6,7
byte lastButtonStatus[2]; // Old status of pins 5,6,7
byte relayStatus[2] = {0, 0}; // Status of relay pins 2,3,4
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
  //Init message
  // Start serial connection
  Serial.begin(57600);;
  
  // Configure input pins and output pins
  for(int i=0; i<sizeof(relayPin); i++)
  {
    pinMode(relayPin[i], OUTPUT); // Configure pin2,3,4 as an output 
    pinMode(buttonPin[i], INPUT_PULLUP); // Configure pin5,6,7 as an pull-up input
    relayStatus[i] = 0; // status of relay is OFF
    digitalWrite(relayPin[i], LOW); //init OFF all relay
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
        Serial.print("Relay ");
        Serial.print(i);
        Serial.print(" has changed status to: ");
        Serial.println(relayStatus[i]); 
      }
    }
    lastButtonStatus[i] = reading;
  }
    
  
  if ( radio.available() )
  {
      printf("Data comes");
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
          boolean isMsgValid = true;
          for(int i=0; i<sizeof(relayPin); i++) {
            if (msg.data[i] != 0 && msg.data[i] != 1) {
              // invalid data received
              printf("Data received is invalid");
              isMsgValid = false;
              break;
            }
          }
          if (isMsgValid) {
            printf("Data received is valid");
            for(int i=0; i<sizeof(relayPin); i++) {
              relayStatus[i] = msg.data[i];
            }
            buttonChangeFlag = true;
          }
        }
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
  
    // Control relay and send latest status of switch board
    if( buttonChangeFlag )
    {
      for (byte i=0; i<sizeof(relayPin); i++) 
      {
        digitalWrite(relayPin[i], relayStatus[i]);
      }
      
      // After Switchboard implement command, send latest status to Gateway
      printf("\nSend updated status due to button status changed");
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

