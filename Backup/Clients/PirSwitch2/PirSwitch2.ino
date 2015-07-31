#include <SPI.h>
#include <EEPROM.h>
#include <NewPing.h>
#include "printf.h"
#include "smarthome.h"

#define DEVICE_TYPE device_pirswitch
#define DEVICE_ID 21

#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 90 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define PIR_PIN  6  // PIR pin

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


const byte relayPin = 11;
const byte ledPin = 13;
const int pirDelay = 15000;

unsigned long lastCheckTime = 0;
boolean switchOn = false;

void setup()
{
  //
  // Print preamble
  //
  Serial.begin(57600);
  pinMode(PIR_PIN, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(relayPin, LOW); // init OFF all relay
  digitalWrite(ledPin, LOW); // init OFF all relay
  switchOn = false; // status of relay is OFF 
  // Initial Health Check Cycle
  lastCheckTime = millis();
}
void loop()
{
  // turn off light if no motion for long time
  if (switchOn) {
    unsigned long lapsedTime = (unsigned long)(millis() - lastCheckTime);
    if (lapsedTime > pirDelay) {
      Serial.println("No motion for long time. Turn off light now.");
      digitalWrite(relayPin, LOW);
      digitalWrite(ledPin, LOW);
      switchOn = false;
    }
  }
  
  int motion = digitalRead(PIR_PIN);
  
  Serial.print("Motion: "); Serial.println(motion);
  
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  unsigned int distance = uS / US_ROUNDTRIP_CM;
  Serial.print("Ping: ");
  Serial.print(distance); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  
  // check if motion detected
  if (motion == HIGH || (distance > 0&&distance<80))
  {
    lastCheckTime = millis();

    if (!switchOn) {
      Serial.println("Motion detected so turn on the light");
      digitalWrite(relayPin, HIGH);
      digitalWrite(ledPin, HIGH);
      switchOn = true;
    }
  }
  delay(60);
}
