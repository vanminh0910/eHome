#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include "Common.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        999    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************

#define ONBOARD_LED     9  // Moteinos have LEDs on D9

#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    115200
#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif


byte msgBuf[MSG_BUF_SIZE];

// RFM69 configuration
RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
Payload data;

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 5000;           // interval at which to blink (milliseconds)

void setup() {
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\neSwitch node starting. Working at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  pinMode(ONBOARD_LED, OUTPUT);
}

void loop() {
  if (radio.receiveDone())
  {
    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else {
      if (radio.ACKRequested()) {
        radio.sendACK();
        Serial.println(" - ACK sent"); 
      }
      Serial.print('Got message from node [');Serial.print(radio.SENDERID, DEC); Serial.print("] ");
      if (promiscuousMode) {
        Serial.print("to [");Serial.print(radio.TARGETID, DEC); Serial.print("] ");
      }
      Serial.println();
      data = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print("Node Id="); Serial.println(data.nodeId);
      for(int i=0; i<4; i++) {
        Serial.print(" data "); Serial.print(i); Serial.print(" = ");
        Serial.println(data.data[i]);
      }      
    }
    Blink(ONBOARD_LED,500);
  }
  
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

    data.nodeId = NODE_ID;

    for(int i=0; i<4; i++) {
      data.data[i] = 1 - data.data[i];
    }
    Serial.print("Data 1: "); Serial.println(data.data[0]);
    Serial.print("Data 2: "); Serial.println(data.data[1]);
    Serial.print("Data 3: "); Serial.println(data.data[2]);
    Serial.print("Data 4: "); Serial.println(data.data[3]);
    if (radio.sendWithRetry(1, (const void*)(&data), sizeof(data)), RETRIES, ACK_TIME)
      Serial.println("Sent ok!");
    else Serial.println("Sent failed...");
    Blink(ONBOARD_LED, 300);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
