#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include "Common.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        2    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************

#define ONBOARD_LED     9  // Moteinos have LEDs on D9

#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    9600
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
        Serial.println(data.data1[i]);
      }
      Serial.print("Data 2 = "); Serial.println(data.data2);
      Serial.print("Data 3 = "); Serial.println(data.data3);      
    }
    Blink(ONBOARD_LED,100);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
