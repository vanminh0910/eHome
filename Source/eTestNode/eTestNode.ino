#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
                      //writeup here: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
#include "Common.h"

#include "DHT.h"

#define DHTPIN 3     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor for normal 16mhz Arduino
//DHT dht(DHTPIN, DHTTYPE);
//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODE_ID        999    //unique for each node on same network

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************

#define ONBOARD_LED     9  // Moteinos have LEDs on D9

//#define SERIAL_EN             //comment this out when deploying to an installed SM to save a few KB of sketch size
#define SERIAL_BAUD    115200
#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif


byte msgBuf[MSG_BUF_SIZE];

SPIFlash flash(5, 0); // flash(SPI_CS, MANUFACTURER_ID)  

// RFM69 configuration
RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
Payload data;

const long InternalReferenceVoltage = 1000;  // Adjust this value to your board's specific internal BG voltage


void setup() {
  Serial.begin(SERIAL_BAUD);
  char buff[50];
  sprintf(buff, "\neTemperature node starting");
  DEBUGln(buff);
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);
  sprintf(buff, "Working at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  pinMode(ONBOARD_LED, OUTPUT);
  // No setup is required for this library
  radio.sleep();
  DEBUGln(radio.readRSSI());
  Blink(ONBOARD_LED, 1000);
}

void loop() {
  // Send status update
  sendUpdate();
  radio.sleep();
  //Serial.flush();
  //delay(2000);
    
  /* 
  Return to sleep state in 5 minutes
  ATmega328 can stay in sleep state longest 8 seconds. So, if we want ATmega328 
  sleep in 5 minutes (5 x 60 = 300 seconds), we need to loop 37 times.
  */
  for(int i = 0; i < 1; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sendUpdate()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = 60; //dht.readHumidity();
  // Read temperature as Celsius
  float t = 33.3; //dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    DEBUGln("Failed to read from DHT sensor!");
    return;
  }

  data.nodeId = NODE_ID;
  data.data[0] = h;
  data.data[1] = t;
  data.data[2] = readVcc();
  data.data[3] = -1;

  DEBUG("Humidity: "); 
  DEBUG(data.data[0]);
  DEBUG(" %\t");
  DEBUG("Temperature: "); 
  DEBUG(data.data[1]);
  DEBUG(" *C ");  
  DEBUG("\t Voltage is: ");
  DEBUGln(data.data[2]);
  
  DEBUG("Sending data to gateway (");
  DEBUG(sizeof(data));
  DEBUG(" bytes) ... ");
  if (radio.sendWithRetry(GATEWAY_ID, (const void*)(&data), sizeof(data)), 0) {
    DEBUG(" Sent ok!");
  } else {
    DEBUG(" Sent failed...");
  }
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
