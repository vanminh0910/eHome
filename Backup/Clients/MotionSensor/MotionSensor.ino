#include "Arduino.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"
#include "LowPower.h"

#define DEVICE_TYPE device_pirswitch
#define DEVICE_ID 55
#define SEND_REPEAT 2

#define LED 4  // Our board have LEDs on D3&D4
#define SERIAL_BAUD   9600
#define MOTIONPIN     1 //hardware interrupt 1 (D3)

// Set up nRF24L01 radio on SPI bus plus pins 8 & 7
RF24 radio(8, 7);

volatile boolean motionDetected=false;
 
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void sendUpdate()
{  
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = MSG_Update;
  msg.data[0] = 1;
  msg.data[1] = DEVICE_ID;
  msg.data[2] = readVcc();
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
    delay(150);
  }
     
  radio.powerDown();
}

void setup() 
{
  Serial.begin(SERIAL_BAUD);
    
  //
  // Print preamble
  //
  
  printf_begin();
  
  //
  // Setup and configure rf radio
  //	
 	
  radio.begin();

  // Optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

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
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  Serial.flush();
  pinMode(LED, OUTPUT); 
  attachInterrupt(MOTIONPIN, motionIRQ, RISING);
}

void motionIRQ()
{
  motionDetected=true;
}


void loop() 
{
  if (motionDetected)
  {
    printf("\nInterrupted. Motion detected.");
    // Send status update
    sendUpdate();
    Serial.println("Sent to gateway");
    Blink(LED, 3);
    motionDetected=false;
    Serial.flush();
  }
  
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
