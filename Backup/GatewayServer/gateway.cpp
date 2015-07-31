#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "MQTTAsync.h"
#include "MQTTClient.h"

#include "smarthome.h"
#include "RF24.h"
#include "list.h"
#include "RCSwitch.h"

#define ADDRESS     "tcp://localhost:1883"
#define PUBCLIENTID    "ClientPub"
#define SUBCLIENTID    "ClientSub"
#define TOPIC    "IN/+/+/+"
#define QOS         2
#define TIMEOUT     3000L
#define MAX_DEVICE_ID 254

// This pin is not the first pin on the RPi GPIO header!
// Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
// for more information.
#define RF315PIN 2

int subscribed = 0;
int subDisconnected = 0;

//Subscribe client
MQTTAsync subClient;

// List of command messages received from openhab server to send
// to clients using RF24
List<struct message> *toSendList = new List<struct message>();

RCSwitch mySwitch;
 

//
// Hardware configuration
//
RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CE,only CE is NEEDED in RPI

////////////// Declare Function /////////////////////////
int sendCommand(struct message msg);
void getServerCommand();

void subConnLost(void *context, char *cause)
{
	subDisconnected = subscribed = 0;

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nSubscribe connection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(subClient, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe connection, return code %d\n", rc);
	}
}

int msgArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *ms)
{
    int i;
    char* payloadptr;
	
	char data[100];
	char topic[100];

    payloadptr = (char*)(ms->payload);
	for(i=0; i<ms->payloadlen; i++)
    {
		data[i] = *payloadptr++;
    }
	data[i] = '\0';
	
	for(i=0; i<topicLen; i++)
    {
		topic[i] = *topicName++;
    }
	topic[i] = '\0';
	
	strcpy(topicName, topic);

	printf("\n\nMessage received. Topic: %s, data: %s", topic, data);fflush(stdout);
	
	struct message msg;
	for(i=0; i<(sizeof(msg.data)/sizeof(msg.data[0])); i++) {
		msg.data[i] = -1;
	}
	char * pch;
	//printf ("\nSplitting topic \"%s\" into tokens:\n", topic);
	int deviceID = 4;
	int msgType = 0;
	int dataIndex = 0;

	// TOPIC format: IN/DeviceID/MsgType/DataIndex
	pch = strtok (topic, "/"); //IN
	pch = strtok (NULL, "/"); //Device ID
	if (pch != NULL) {
		deviceID = atoi(pch);
		printf("\nDevice ID: %d", deviceID);
	}
	pch = strtok (NULL, "/"); //Message type
	if (pch != NULL) {
		msgType = atoi(pch);
		printf("\nMessage Type: %d", msgType);
	}
	pch = strtok (NULL, "/"); //Data index
	if (pch != NULL) {
		dataIndex = atoi(pch);
		printf("\nData index: %d", dataIndex);
		if (dataIndex<0 || dataIndex>=(sizeof(msg.data)/sizeof(msg.data[0]))) {
			printf("\nInvalid data index");
			return 1;
		}
	}

	msg.deviceID = deviceID;
	msg.msgType = msgType;
	msg.data[dataIndex] = atoi(data);
	printf("\nData: %d", msg.data[dataIndex]);
	msg.deviceType = 0;
	
	// add to RF24 message queued list
	toSendList->push_back(msg);

    MQTTAsync_freeMessage(&ms);
    MQTTAsync_free(topicName);
    return 1;
}


void onSubDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successfully disconnect subscribe client\n");
	subDisconnected = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
	printf("Subscribe succeeded\n");
	subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
}


void onSubConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe client failed to connect, rc %d\n", response ? response->code : 0);
	subDisconnected = 1;
}


void onSubConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	printf("Successful subscribe connection\n");

	printf("Subscribing to all topics for client %s using QoS %d\n\n",
		SUBCLIENTID, QOS);
		
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;

	if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		exit(-1);
	}
}

int initializeSubClient()
{
	int rc;
	
	MQTTAsync_connectOptions subConnOpts = MQTTAsync_connectOptions_initializer;
	
	MQTTAsync_create(&subClient, ADDRESS, SUBCLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(subClient, NULL, subConnLost, msgArrived, NULL);

	subConnOpts.keepAliveInterval = 20;
	subConnOpts.cleansession = 1;
	subConnOpts.onSuccess = onSubConnect;
	subConnOpts.onFailure = onSubConnectFailure;
	subConnOpts.context = subClient;
	if ((rc = MQTTAsync_connect(subClient, &subConnOpts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(-1);
	}

	while	(!subscribed)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	
	return 0;
}

int disconnectSubClient()
{
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	opts.onSuccess = onSubDisconnect;
	opts.context = subClient;
	int rc;

	if ((rc = MQTTAsync_disconnect(subClient, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to disconnect subscriber client, return code %d\n", rc);
		return rc;
	}
	return 0;
}

void setup(void)
{
	// Start RF24 hardware
	radio.begin();

	// optionally, increase the delay between retries & # of retries
	radio.setRetries(15, 15);

	// optionally, reduce the payload size.  seems to
	// improve reliability
	radio.setPayloadSize(sizeof(message));
	radio.setChannel(0x4c);
	radio.setPALevel(RF24_PA_MAX);
	radio.setDataRate(RF24_250KBPS);
	radio.setAutoAck(0);

	//
	// Open pipes for clients to communicate
	//
	// Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
	radio.openWritingPipe(CLIENT_PIPE);
	radio.openReadingPipe(1, GATEWAY_PIPE);

	//
	// Start listening
	//
	radio.startListening();

	//
	// Dump the configuration of the rf unit for debugging
	//
	radio.printDetails();
	
	// subscribe to MQTT Broker
	if (initializeSubClient() != 0)
	{
		printf("Failed to start subscriber client\n");
		exit(-1);
	}
	
	//initialize RF receiver
	if(wiringPiSetup() == -1)
       return 0;

    mySwitch = RCSwitch();
    mySwitch.enableReceive(RF315PIN);  // Receiver on interrupt 0 => that is pin #2
}

void receiveStatus(void)
{
	// if there is data ready
    if ( radio.available() )
    {
		printf("\nReceiving RF24 message received from client");
        // Dump the payloads until we've gotten everything
        struct message msg;
        bool done = false;
        while (!done)
        {
            // Fetch the payload, and see if this was the last one.
            done = radio.read( &msg, sizeof(struct message));
        }

		// Spew it
        printf("\nMessage: msgType - %d;", msg.msgType);
        printf("\nDeviceID - %d", msg.deviceID);
        printf("\nDeviceType - %d;", msg.deviceType);
        printf("\ndata1: %d", msg.data[0]);
        printf("\ndata2: %d", msg.data[1]);
        printf("\ndata3: %d", msg.data[2]);
		printf("\ndata4: %d", msg.data[3]);
		
		// validate received data
		if (msg.deviceID < 0 || msg.deviceID > MAX_DEVICE_ID) {
			printf("\nData received is invalid\n");
			return;
		}		

		// send message to broker		
		MQTTClient client;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		MQTTClient_message pubmsg = MQTTClient_message_initializer;
		MQTTClient_deliveryToken token;
		int rc;

		MQTTClient_create(&client, ADDRESS, PUBCLIENTID,
			MQTTCLIENT_PERSISTENCE_NONE, NULL);
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;

		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			printf("Failed to connect to broker to publish, return code %d\n", rc);
			return;
		}
		
		for(int i=0; i<sizeof(msg.data)/sizeof(msg.data[0]); i++) {
			if (msg.data[i] < 0) {
				// this field contains invalid data so skip it
				continue;
			}
			char content[50];
			char topic[50];
			sprintf(content, "%d", msg.data[i]);
			pubmsg.payload = content;
			pubmsg.payloadlen = strlen(content);
			pubmsg.qos = QOS;
			pubmsg.retained = 0;
			// now save the sent message to history to resolve loop issue
			// see comment in the beginning of this code file			
			sprintf(topic, "OUT/%d/%d/%d", msg.deviceID, msg.msgType, i);
			
			MQTTClient_publishMessage(client, topic, &pubmsg, &token);
			//printf("\nWaiting for up to %d seconds for publication of message \n"
			//		"on topic %s with data %s\n",
			//		(int)(TIMEOUT/1000), mqttMsg.topic, mqttMsg.message);
			rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
			printf("\nMessage with delivery token %d delivered", token);
		}
		
		MQTTClient_disconnect(client, 10000);
		MQTTClient_destroy(&client);
    }
}

int sendCommand(struct message msg)
{
	// Send command
	
	// First, stop listening so we can talk.
	radio.stopListening();
	fflush(stdout);
	
	radio.openWritingPipe(CLIENT_PIPE + msg.deviceID);

	// Take the time, and send it.  This will block until complete
	bool ok = radio.write( &msg, sizeof(msg) );
	
	if(ok)
	  printf("\nSend command to client successfully!");
	else
	  printf("\nSend command failed!");

	// Now, continue listening
	radio.startListening();
	return 0;
}

void printRF24Message(const struct message m)
{
	printf("\nDevice ID = %d", m.deviceID);
	printf("\nMessage type = %d", m.msgType);
	for (int i=0; i<sizeof(m.data)/sizeof(m.data[0]); i++) {
		printf("\nData %d = %d", i, m.data[i]);
	}
}

void sendMQTTMessage(char* top, char* mes)
{
	// send message to broker		
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	int rc;

	MQTTClient_create(&client, ADDRESS, PUBCLIENTID,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect to broker to publish, return code %d\n", rc);
		return;
	}
	
	pubmsg.payload = mes;
	pubmsg.payloadlen = strlen(mes);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	
	MQTTClient_publishMessage(client, top, &pubmsg, &token);
	//printf("\nWaiting for up to %d seconds for publication of message \n"
	//		"on topic %s with data %s\n",
	//		(int)(TIMEOUT/1000), top, mes);
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	printf("\nMessage with delivery token %d delivered", token);
	
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	return;
}

int main(int argc, char** argv)
{
	printf("\nStarting gateway server\n");
	setup();

	while(1) {
		// Send all queued message to clients using RF24
		Node<struct message>* mp = NULL;
		while (!toSendList->empty()) {
			printf("\nSending RF24 message in queue");
			mp = toSendList->pop_back(); 
			printRF24Message((struct message)mp->value);
			fflush(stdout);
			//sendCommand((struct message)mp->value);
			//printf("\nDeleted node");
			fflush(stdout);
			usleep(200);
			delete mp;
		}
		// check RF24 radio available
		receiveStatus();
		
		// check RF315Mhz radio available
		/*
		if (mySwitch.available()) {
    
			int value = mySwitch.getReceivedValue();
		
			if (value == 0) {
			  printf("\nRf code using unknown encoding");
			} else {    
				printf("\nRf signal received: %d\n", value);
				
			}
			char buffer [33];
			sprintf(buffer, "%d", value);
			sendMQTTMessage("RF", buffer);
			mySwitch.resetAvailable();		
		}*/

		fflush(stdout);
		usleep(2000);
	}
	
	delete toSendList;
	return 0;
}