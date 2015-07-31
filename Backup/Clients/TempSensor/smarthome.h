/**
 * File: smarthome.h
 *
 * Description: define command, pipe address, structure use in smarhome system
 * 
 */

#ifndef __SMARTHOME_H__
#define __SMARTHOME_H__

////////////////// COMMON DEFINE ///////////////////////////////////
#define ON                        1
#define OFF                       0
#define GATEWAY_ID                0x0
#define BROADCAST_ID              0xff


////////////////// PIPE ADDRESS ////////////////////////////////////
#define GATEWAY_PIPE              0xF0F0F0F0A0LL
#define CLIENT_PIPE               0xF0F0F0F0B0LL


////////////////// COMMNUNICATION COMMAND //////////////////////////
enum message_type
{
	MSG_Register = 1, //Register message, sent when client boots up
	/*Update message sent to gateway when client status changed, 4 data int used for relay status
	  With sensor, only 1 or 2 data int used
	  With door lock, only 1 data int used for open or locked status
	*/
	MSG_Update,
	/*
	  Require Update message sent by gateway when booting up.
	  When received this, client must send its status to GW
	*/
	MSG_RequireUpdate,
	/*
	  Change Status message, sent from gateway to client to ask for changing status
	  With switch board or PIR, first byte is relay to turn on or off (1-4), second int is 0 (OFF) or 1 (ON)
	  With door lock, first int is used, either 0 (LOCKED) or 1 (OPEN)

	*/
	MSG_ChangeStatus,

        /*
	  Send setting message, sent from gateway to client to ask for changing setting values
	  For example, with PIR, first byte is cds threshold (0-255), second int is PIR delay (0-200)
	*/
	MSG_SendSettings

};


////////////////// DEVICE TYPE ///////////////////////////////////
enum device_type
{
  device_switchboard = 10,
  device_pirswitch = 20,
  device_tempsensor = 51,
  device_soilsensor = 52,
  device_gassensor = 53,
  device_pirsensor = 55,
  device_lockdoor = 60
};


////////////////// STRUCTURE ///////////////////////////////////////
// Message Structure 
typedef struct message
{
  uint8_t msgType;
  uint8_t deviceType;
  uint8_t deviceID;
  uint16_t data[4];
};

#endif // __PRINTF_H__

