// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
#define RELAY_ON 1
#define RELAY_OFF 0

const int numReadings = 5;
const int light_thres = 210;
const unsigned long max_time = 120000; // max idle time for light without motion detected ~ 3 mins

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int light_pin = A6;
int pir_pin = 9;
int relay_pin = 4;
int led_pin = 13;
boolean relay_on = false;

unsigned long last_motion = 0;

void setup() {
  pinMode(pir_pin, INPUT);
  pinMode(light_pin, INPUT);
  pinMode(relay_pin, OUTPUT);
  pinMode(led_pin, OUTPUT);
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0; 
  relay_on = false;
  digitalWrite(relay_pin, RELAY_OFF);
  Serial.begin(9600);

}

void loop() {
  // get light sensore value
  // subtract the last reading:
  total = total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(light_pin); 
  // add the reading to the total:
  total = total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;                           

  // calculate the average:
  average = total / numReadings;
  Serial.print("Light: "); Serial.println(average);
  
  if (average < light_thres) { // if the light is dark
    // check if motion is detected or not
    int motion = digitalRead(pir_pin);
    Serial.print("Motion: "); Serial.println(motion);
    digitalWrite(led_pin, motion);
    if (motion) { // motion detected
      if (!relay_on) { // light is current off, then turn it on
        Serial.println("Motion detected. Light is current off, then turn it on");
        relay_on = true;
        digitalWrite(relay_pin, RELAY_ON);
      }
      last_motion = millis(); //save last time motion is detected
    }
  }
  
  if (relay_on) { // check the time to turn off when light is on
    unsigned long current = millis();
    if (current - last_motion > max_time) {
      Serial.println("No motion detected long time. Turn light off");
      relay_on = false;
      digitalWrite(relay_pin, RELAY_OFF);
    }
  }
  
  delay(500);
}
