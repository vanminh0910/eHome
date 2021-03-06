#ifndef COMMON_H
#define COMMON_H

/* Common header for eHome system 

written by Minh Ha 2015
*/

#define NETWORK_ID     77  //the same on all nodes that talk to each other
#define GATEWAY_ID     1

#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ

#define ENCRYPTKEY    "eHomeEncryptKey" //exactly the same 16 characters/bytes on all nodes!

#define ACK_TIME      90 // max # of ms to wait for an ack

#define RETRIES  2

#define MSG_BUF_SIZE 18   // message buffer size: 2-byte NODEID + 4x4-bytes for float

#define SERIAL_BUF_SIZE 50   // message buffer size for reading serial input

typedef struct {		
  int nodeId; //store this nodeId
  float data[4];
} Payload;

#endif

