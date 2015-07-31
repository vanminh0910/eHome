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
#define START_BYTE                0x55
#define STOP_BYTE                 0x77
#define GATEWAY_ID                0x0
#define BROADCAST_ID              0xff


////////////////// PIPE ADDRESS ////////////////////////////////////
#define GATEWAY_PIPE              0xF0F0F0F0A0LL
#define CLIENT_PIPE               0xF0F0F0F0B0LL


////////////////// COMMNUNICATION COMMAND //////////////////////////
enum command
{
  cmd_set_status = 1,
  cmd_request_status,
  cmd_update_status,
  cmd_register 
};


////////////////// DEVICE TYPE ///////////////////////////////////
enum device_type
{
  device_switchboard = 1,
  device_tempsensor,
  device_lockdoor,
  device_curtain
};


////////////////// ROOM LIST /////////////////////////////////////
enum room_list
{
  living_room = 1,
  kitchen_room,
  bed_room_1,
  bed_room_2,
  bed_room_3,
  bath_room_1,
  bath_room_2
};

enum message_type
{
	Register = 1, //Register message, sent when client boots up
	/*Update message sent to gateway when client status changed, 4 data int used for relay status
	  With sensor, only 1 or 2 data int used
	  With door lock, only 1 data int used for open or locked status
	*/
	Update,
	/*
	  Require Update message sent by gateway when booting up.
	  When received this, client must send its status to GW
	*/
	RequireUpdate,
	/*
	  Change Status message, sent from gateway to client to ask for changing status
	  With switch board or PIR, first byte is relay to turn on or off (1-4), second int is 0 (OFF) or 1 (ON)
	  With door lock, first int is used, either 0 (LOCKED) or 1 (OPEN)

	*/
	ChangeStatus,

        /*
	  Send setting message, sent from gateway to client to ask for changing setting values
	  With PIR, first byte is cds threshold (0-255), second int is PIR delay (0-200)
	*/
	SendSettings

};



////////////////// STRUCTURE ///////////////////////////////////////
// Message Structure 
typedef struct message
{
  uint32_t msgType;
  uint32_t deviceType;
  uint32_t deviceID;
  uint32_t data[6];
};

// Client data
typedef struct client
{
  char id;
  char type;
  char room;
  char isAlive;
  char data[3];
};

#endif // __PRINTF_H__

