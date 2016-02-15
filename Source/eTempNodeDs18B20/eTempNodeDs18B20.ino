#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
                      //writeup here: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Common.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        3    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************
#define ONBOARD_LED     9  // Moteinos have LEDs on D9

//#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    115200
#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(0);}
#define DEBUGln(input) {Serial.println(input); delay(0);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif

#define BATT_CYCLES   30  //read and report battery voltage every this many wakeup cycles (ex 30cycles * 8sec sleep = 240sec/4min)

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3
// Sensor can be powered by a digital pin
#define SENSOR_VCC 4
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// RFM69 configuration
RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
Payload data;

float batteryVolts = -1;
byte cycleCount = BATT_CYCLES;


void setup() {
  Serial.begin(SERIAL_BAUD);
  char buff[50];
  sprintf(buff, "\neTemperature node starting. ");
  DEBUGln(buff);
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);
  sprintf(buff, "\nWorking at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(SENSOR_VCC, OUTPUT);  
  
  DEBUGln("Starting sensor");
  sensors.begin();
  DEBUGln("Setup done");
}

void loop() {
  // Turn on sensor power pin
  //digitalWrite(SENSOR_VCC, HIGH);
  //delay(10);
  sendUpdate();
  // Turn off sensor power pin to save battery
  //digitalWrite(SENSOR_VCC, LOW);
  //Serial.flush();
  radio.sleep();
    
  /* 
  Return to sleep state in 5 minutes
  ATmega328 can stay in sleep state longest 8 seconds. So, if we want ATmega328 
  sleep in 5 minutes (5 x 60 = 300 seconds), we need to loop 37 times.
  */
  for(int i = 0; i <37; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sendUpdate()
{
  // Issue a global temperature
  // request to all devices on the bus
  DEBUG(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures

  DEBUGln("Temperature for Device 1 is: ");
  float temparature = 0;
  float temp = 0;
  for (int i=0; i<5; i++) {
    temp = sensors.getTempCByIndex(0);
    if (isnan(temp)) {
      DEBUGln("Failed to read from sensor!");
      return;
    }
    temparature += temp;
    delay(50);
  }
  temparature = temparature/5;
  DEBUGln(temparature);
  
  checkBattery();

  // Build the data payload and send to gateway
  data.nodeId = NODE_ID;
  data.data[0] = temparature;
  data.data[1] = -1;
  data.data[2] = -1;
  data.data[3] = batteryVolts;

  DEBUG("\t Voltage is: ");
  DEBUGln(data.data[3]);
  
  DEBUG("Sending data to gateway (");
  DEBUG(sizeof(data));
  DEBUG(" bytes) ... ");
  digitalWrite(ONBOARD_LED, HIGH);
  if (radio.send(GATEWAY_ID, (const void*)(&data), sizeof(data)), 0) {
    DEBUG(" Sent ok!");
  } else {
    DEBUG(" Sent failed...");
  }
  digitalWrite(ONBOARD_LED, LOW);
  DEBUGln();
  Blink(ONBOARD_LED, 50); 
}

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

void checkBattery()
{
  if (cycleCount++ == BATT_CYCLES) //only read battery every BATT_CYCLES sleep cycles
  {
    long readings=0;
    for (byte i=0; i<10; i++) //take 10 samples, and average
      readings += readVcc();
    batteryVolts = readings / 10.0;
    cycleCount = 0;
  } else {
    batteryVolts = -1;
  }
}
