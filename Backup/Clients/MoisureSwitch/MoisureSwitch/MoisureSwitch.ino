#include <SPI.h>
#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

#define DEVICE_ID 88
#define DEVICE_TYPE 80
#define UPDATE_INTERVAL 300000
#define CHECK_TIME 1000
#define WATER_TIME 1000

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(9, 10); 

const int analogInPin = A3;   // Analog input pin of moisuer sensor
int sensorValue = 0;          // value read from the sensor
int dry_point = 0;            // Store a standard value of dry status
long delay_value = 0;
int relayPin = 4;
unsigned long lastCheckTime = 0;
unsigned long checkTime = 0;

void sendUpdate()
{
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = 2;
  msg.data[0] = sensorValue;
  msg.data[1] = dry_point;
  msg.data[2] = 0;
  
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
    
  // initialize the moisure signal.
  dry_point = 450;  
  pinMode(relayPin, OUTPUT); // Configure pin2,3,4 as an output 
  digitalWrite(relayPin, LOW); // init OFF all relay
  delay_value = 0;
  checkTime = millis();
  
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
}
void loop()
{ 
  //Check moisure sensor
  // read the analog in value:
  sensorValue = analogRead(analogInPin);    
  if((millis() - checkTime) > CHECK_TIME )      // Send Update Status after 1 minutes
  {
    sendUpdate();
    // Update Health Check Time
    checkTime = millis();  
    
    if(sensorValue > dry_point){
      // Water
      Serial.println("Water..." );
      digitalWrite(relayPin, HIGH);
      delay(WATER_TIME);
      Serial.println("Done!" );
      digitalWrite(relayPin, LOW);      
    }
    
    // print the results to the serial monitor:
    Serial.print("sensor analog = " );                       
    Serial.println(sensorValue); 
  }
  
  // Get command form server
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
          // Water
          Serial.println("Water..." );
          digitalWrite(relayPin, HIGH);
          delay(WATER_TIME);
          Serial.println("Done!" );
          digitalWrite(relayPin, LOW);          
      } else if (msg.msgType == DailyWater){ 
        if(sensorValue < dry_point){
          // Water
          Serial.println("Water..." );
          digitalWrite(relayPin, HIGH);
          delay(WATER_TIME);
          Serial.println("Done!" );
          digitalWrite(relayPin, LOW);       
        }
      } else if (msg.msgType == SendSettings) {
        dry_point = msg.data[0];
        sendUpdate();
        // Update Health Check Time
        lastCheckTime = millis();
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

  // Set check time
  if(checkTime > millis())                                // Timer of Arduino resets after 50 days
  {
    checkTime = millis();
  }

}
