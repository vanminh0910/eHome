/*
 * IRSender: sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 Feb, 2016
 * Copyright 2016 Minh Ha
 * http://xyz.com
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>


// Update these with values suitable for your network.

const char* ssid = "Sandiego";
const char* password = "0988807067";
const char* mqtt_server = "192.168.1.110";
const char* topic = "/IR1/+";
const char* topic_tv = "/IR1/1";
const char* topic_ac = "/IR1/2";
const char* topic_fan = "/IR1/3";

unsigned int  fanCode[51] = {3450,3450, 950,2500, 950,2500, 850,850, 900,2550, 950,2500, 900,2550, 900,850, 950,750, 900,850, 850,850, 950,800, 950,2500, 900,850, 950,750, 950,2500, 900,850, 950,750, 950,800, 900,2550, 950,2500, 950,2450, 900,2550, 900,2550, 900,850, 950};  // SANYO DC123E
unsigned int  acOnCode[211] = {3850,1800, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,400, 500,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,400, 500,1350, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,1350, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,1300, 550,400, 500,400, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,1350, 550,1300, 550,1300, 550,1350, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 500,400, 550};  // PANASONIC 555A:F3080088
unsigned int  acOffCode[211] = {3850,1800, 550,350, 550,1350, 550,350, 550,1350, 500,400, 550,1300, 600,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,350, 550,1350, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,400, 550,1350, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 500,450, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 500,400, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550};

WiFiClient espClient;
PubSubClient client(espClient);
IRsend irsend(0); //an IR led is connected to GPIO pin 0

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  irsend.begin();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, topic_tv) == 0) {
    Serial.println("TV signal");
    if ((char)payload[0] == '0') {
      Serial.println("TV signal OFF");
      for (int i = 0; i < 5; i++) {
        irsend.sendSAMSUNG(0xE0E040BF, 32);
        delay(40);
      }
    } else if ((char)payload[0] == '1') {
      Serial.println("TV signal ON");
      for (int i = 0; i < 5; i++) {
        irsend.sendSAMSUNG(0xE0E040BF, 32);
        delay(40);
      }
    } 
  } else if (strcmp(topic, topic_ac) == 0) {
    Serial.println("AC signal");
    if ((char)payload[0] == '0') {
      Serial.println("AC signal OFF");
      for (int i = 0; i < 5; i++) {
        //irsend.sendPanasonic(0x555A, 0xF3080084);
        irsend.sendRaw(acOffCode, sizeof(acOffCode) / sizeof(acOffCode[0]), 38);
        delay(40);
      }
    } else if ((char)payload[0] == '1') {
      Serial.println("AC signal ON");
      for (int i = 0; i < 5; i++) {
        //irsend.sendPanasonic(0x555A, 0xF3080088);
        irsend.sendRaw(acOnCode, sizeof(acOnCode) / sizeof(acOnCode[0]), 38);
        delay(40);
      }
    }
  } else if (strcmp(topic, topic_fan) == 0) {
    Serial.println("Fan signal");
    if ((char)payload[0] == '0') {
      Serial.println("Fan signal OFF");
      for (int i = 0; i < 5; i++) {
        irsend.sendRaw(fanCode, sizeof(fanCode) / sizeof(fanCode[0]), 38);
        delay(40);
      }
    } else if ((char)payload[0] == '1') {
      Serial.println("Fan signal ON");
      for (int i = 0; i < 5; i++) {
        irsend.sendRaw(fanCode, sizeof(fanCode) / sizeof(fanCode[0]), 38);
        delay(40);
      }
    }
  } else {
    Serial.println("Unknown signal");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266IRSender1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
