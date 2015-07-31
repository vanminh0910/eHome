/**
 * Gateway Control Sample Code
 * 
 * This is a gateway server code to communicate with client devices
 */
#include <SD.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

/////////////// Define //////////////////////////////////
#define MQTT_SERVER "192.168.1.110"
#define MQTT_CLIENTID "arduinoClient0910"
#define MQTT_TOPIC "IN/+/+/+"
#define SEND_REPEAT 2
#define MAX_BUTTON    12
#define PIN_VT             3   
#define PIN_D0             5
#define PIN_D1             6
#define PIN_D2             7
#define PIN_D3             8

////////////// Declare Function /////////////////////////
void SendCommand(message m);
void ProcessMessage(message m);
void callback(char* topic, byte* payload, unsigned int length);


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(7,8);
//RF24 radio(48,49);      // Use for atmega 2560


////////////// Declare global variable //////////////////
uint64_t lastCheckTime;              // Last time point to check clients connect or disconnect with system.
uint8_t D0, D1, D2, D3;          // storage for data states from RF remote
bool stopFlag = false;

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server[] = { 192, 168, 1, 110 };
byte ip[]     = { 192, 168, 1, 120 };
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup(void)
{ 
  Serial.begin(57600);
  printf_begin();

  // Initial Remote Function
  // setup the R06A decoder connections
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  pinMode(PIN_D2, INPUT);
  pinMode(PIN_D3, INPUT);
  pinMode(PIN_VT, INPUT); 
  //stopFlag = false; 
  
  // start the Ethernet connection and the server:
  printf("\nInitializing Ethernet...");

  Ethernet.begin(mac, ip);
  printf("\nInitialized Ethernet port. IP is: ");
  Serial.println(Ethernet.localIP());

  if (client.connect(MQTT_CLIENTID)) {
    // subscribe to topic with pattern IN/DeviceID/MessageType/DataIndex
    client.subscribe(MQTT_TOPIC);
  } else {
    printf("\nCannot connect to MQTT broker");
    return;
  }

  //
  // Setup and configure rf radio
  //	
  	
  radio.begin();

  // Optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);

  // Optionally, reduce the payload size.  seems to
  // Improve reliability
  radio.setPayloadSize( sizeof(struct message) );
  
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(0);

  // Open pipes to other nodes for communication
  radio.openWritingPipe(CLIENT_PIPE);
  radio.openReadingPipe(1, GATEWAY_PIPE);
     
  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails(); 
}

//**** loop Function ****
// This function is used to get request from user and receive a message from client
void loop(void)
{
  //////////////////////////////////////////////////////////  
  // Get request from User with Serial Interface
  //////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////  
  // Connect to MQTT broker if not connected or disconnected
  //////////////////////////////////////////////////////////
  if (!client.connected()) {
      if (client.connect(MQTT_CLIENTID)) {
        // subscribe to topic with pattern IN/DeviceID/MessageType/DataIndex
        client.subscribe(MQTT_TOPIC);
        printf("\nConnected and subscribed topic to MQTT broker");
      } else {
        printf("\nCannot connect to MQTT broker");
        return;
      }
  }
  client.loop();
  //////////////////////////////////////////////////////////////////
  // Get message from client
  //////////////////////////////////////////////////////////////////
  if(radio.available())
  {
    printf("\nRF24 message coming");
    // Read message
    message msg;
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &msg, sizeof(struct message) );  
    }
    // Analyze message
    ProcessMessage(msg);
  } 

  return;  
  //////////////////////////////////////////////////////////////////
  // Remote Function
  //////////////////////////////////////////////////////////////////
  // Read status pin VT of receive module
  if (digitalRead(PIN_VT) && !stopFlag)
  {
    /*
    Remote button map 
      button 1 : D0
      button 2 : D1
      button 3 : D0,D1
      button 4 : D2
      button 5 : D0,D2
      button 6 : D1,D2
      button 7 : D0,D1,D2
      button 8 : D3
      button 9 : D0,D3
      button 10: D1,D3
      button 11: D0,D1,D3
      button 12: D2,D3
    */
    D0 = digitalRead(PIN_D0);    
    D1 = digitalRead(PIN_D1);
    D2 = digitalRead(PIN_D2);
    D3 = digitalRead(PIN_D3);
    stopFlag = true;
    uint8_t remoteButton = D0 | (D1 << 1) | (D2 << 2) | (D3 << 3);
    printf("\nButton pressed: %d", remoteButton);
    if((remoteButton > 0) && (remoteButton < 13))
    {
      //TODO: handle remote button here
    }
  }
  else if (!digitalRead(PIN_VT) && stopFlag)
  {
    stopFlag = false;     
  }  
}



void SendCommand(message msg)
{
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
  radio.openWritingPipe(CLIENT_PIPE+msg.deviceID);
  for (int i=0; i<SEND_REPEAT; i++) {
    bool ok = radio.write( &msg, sizeof(struct message));
    if (ok)
      printf("\nSend command ok...");
    else
      printf("\nSend command failed.");
    delay(150);
  }
  // Now, resume listening so we catch the next packets.
  radio.startListening(); 
}

//**** ProcessMessage Function ****
// This function is used to analyze received message and give a compatible respond
void ProcessMessage(message m)
{
  printf("\nMsg size: %d", sizeof(struct message));
  printf("\nMsg Type type: %d", m.msgType);
  printf("\nDevice ID: %d", m.deviceID);
  printf("\nDevice type: %d", m.deviceType);
  printf("\nData 1: %d", m.data[0]);
  printf("\nData 2: %d", m.data[1]);
  printf("\nData 3: %d", m.data[2]);
  printf("\nData 4: %d", m.data[3]);
  printf("\n\n");

  // parse the RF24 message and publish as MQTT message
  for(int i=0; i<sizeof(m.data)/sizeof(m.data[0]); i++) {
    char topic[50];
    char msg[50];
    sprintf(topic, "OUT/%d/%d/%d", m.deviceID, m.msgType, i);
    sprintf(msg, "%d", m.data[i]);
    client.publish(topic, msg);
    printf("\nPublished message %s to topic %s", msg, topic);
    delay(25);
  }
}

// handles MQTT message arrived on subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  char msgBuffer[50];
  int i = 0;
  Serial.println("\n\nMQTT message arrived.  Topic: " + String(topic));
  Serial.println("Length: " + String(length, DEC));
  
  String topicString = String(topic);
    
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    msgBuffer[i] = payload[i];
  }
  msgBuffer[i] = '\0';
  
  String msgString = String(msgBuffer);
  
  Serial.println("Payload: " + msgString);
  
  // Parse topic. Topic should has pattern like IN/DeviceID/MsgType/DataIndex
  message m;
  int index1 = topicString.indexOf("/");
  int index2 = topicString.indexOf("/", index1+1);
  int index3 = topicString.indexOf("/", index2+1);

  m.deviceID = topicString.substring(index1+1, index2).toInt();
  m.msgType = topicString.substring(index2+1, index3).toInt();
  int dataIndex = topicString.substring(index3+1).toInt();
  m.deviceType = 0; // Device Type field not used for now
  for(i=0; i<sizeof(m.data)/sizeof(m.data[0]); i++) {
    m.data[i] = -1;
  }
  m.data[dataIndex] = msgString.toInt();
  SendCommand(m);
}

