#include "Arduino.h"
#include "Dht11.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"
#include "LowPower.h"

#define DEVICE_ID 51
#define DEVICE_TYPE device_tempsensor
#define SEND_REPEAT 2

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(8, 7);

// The data I/O pin connected to the DHT11 sensor
const byte DHT_DATA_PIN = A3;
const byte BATT_DATA_PIN = A1;
static Dht11 sensor(DHT_DATA_PIN);

const long InternalReferenceVoltage = 1062;  // Adjust this value to your board's specific internal BG voltage
 
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
  sensor.read();
  message msg;
  msg.deviceID = DEVICE_ID;
  msg.deviceType = DEVICE_TYPE;
  msg.msgType = MSG_Update;
  msg.data[0] = sensor.getHumidity();
  msg.data[1] = sensor.getTemperature();
  msg.data[2] = readVcc();  
  
  // Print status Sensor board on Serial Monitor
  printf("------Send update status------\n");
  printf("Humidity    : %d\n", msg.data[0]);
  printf("Temperature : %d\n", msg.data[1]);
  printf("Volt : %d\n", msg.data[2]);

  // Stop listening so we can talk
  radio.stopListening();
  for(int i=0; i<SEND_REPEAT; i++) {
    bool ok = radio.write( &msg, sizeof(struct message));
    //if (ok)
    //  printf("send update ok... \n");
    //else
    //  printf("send update failed.\n\r");
    delay(50);
  }
      
  // Continue listening
  radio.startListening();
}

void setup() 
{
  Serial.begin(57600);
  Serial.print("Dht11 Lib version ");
  Serial.println(Dht11::VERSION);
    
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

  //radio.printDetails();
  
  sendUpdate();
}

void loop() 
{  
  // Send status update
  sendUpdate();
  radio.powerDown();
  Serial.flush();
    
  /* 
  Return to sleep state in 5 minutes
  ATmega328 can stay in sleep state longest 8 seconds. So, if we want ATmega328 
  sleep in 5 minutes (5 x 60 = 300 seconds), we need to loop 37 times.
  */
  for(int i = 0; i < 15; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }
}
