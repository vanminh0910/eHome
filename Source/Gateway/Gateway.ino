
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
                      //writeup here: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
#include "Common.h"
//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
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

char serialData[SERIAL_BUF_SIZE];

RFM69 radio;

bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

Payload data;

void setup() {
  Serial.begin(SERIAL_BAUD);
  char buff[50];
  sprintf(buff, "\nGateway starting");
  DEBUGln(buff);
  radio.initialize(FREQUENCY, GATEWAY_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);
  sprintf(buff, "\nWorking at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  pinMode(ONBOARD_LED, OUTPUT);
  Blink(ONBOARD_LED, 1000);
}

void loop() {
  //process any serial input
  if (Serial.available() > 0) {
    getSerialLine(serialData);
    //Serial.println(serialData);
    //Serial.println(serialData[0]);
    //Serial.println(serialData[strlen(serialData)-1]);
    if (serialData[0] == 'S' && serialData[strlen(serialData)-1] == 'E')
    {
      Serial.println("Enter serial processing");
      char *str;
      char *p = serialData;
      int index = 0;
      int nodeId;
      while ((str = strtok_r(p, ";", &p)) != NULL) { // delimiter is the semicolon
        if (index == 1) {
          nodeId = atoi(str);
          data.nodeId = GATEWAY_ID;
        } else if (index >= 2 && index <= 5) {
          float value = atof(str);
          data.data[index-2] = value;
        }
        index++;
      }
      
      char buf[50];
      sprintf(buf, "Node ID to send: %d", nodeId);
      Serial.println(buf);
      Serial.print("Data 1: "); Serial.println(data.data[0]);
      Serial.print("Data 2: "); Serial.println(data.data[1]);
      Serial.print("Data 3: "); Serial.println(data.data[2]);
      Serial.print("Data 4: "); Serial.println(data.data[3]);
      if (radio.sendWithRetry(nodeId, (const void*)(&data), sizeof(data)), RETRIES, ACK_TIME)
        Serial.println("Sent ok!");
      else Serial.println("Sent failed...");
      Blink(ONBOARD_LED, 300);
    }
  }

  if (radio.receiveDone())
  {
    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else {
      if (radio.ACKRequested()) {
        byte theNodeID = radio.SENDERID;
        radio.sendACK();
        Serial.println(" - ACK sent"); 
      }
      //Serial.println('Got message from node [');Serial.print(radio.SENDERID, DEC); Serial.println("] ");
      //if (promiscuousMode) {
        //Serial.print("to [");Serial.print(radio.TARGETID, DEC); Serial.println("] ");
      //}
      Serial.println();
      data = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      // Send data to Raspberry Pi via serial communication      
      Serial.print("S;"); //start message
      Serial.print(data.nodeId);Serial.print(";");
      for(int i=0; i<4; i++) {
        Serial.print(data.data[i]);
        Serial.print(";");
      }
      Serial.println("E"); //end message
    }
    Blink(ONBOARD_LED,300);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}


void getSerialLine(char * buffer)
{
  memset(buffer, 0, sizeof(buffer));
  uint8_t idx = 0;
  char c;
  while (Serial.available() > 0) { // wait for a char this causes the blocking
    c = Serial.read(); delay(10);
    if (c == '\n' || c == '\r') { 
      buffer[idx] = 0;
      break;
    } else
      buffer[idx++] = c;    
  }  
}
