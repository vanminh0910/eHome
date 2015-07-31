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
#define GATEWAY_PIPE              0xF0F0F0F0E1LL
#define CLIENT_PIPE               0xF0F0F0F0D2LL


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


enum sw_relay_type
{
  sw_undefined = 0,
  sw_light,
  sw_fan,
  sw_tv
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


////////////////// STRUCTURE ///////////////////////////////////////
// Message Structure 
typedef struct message
{
  uint8_t startByte;
  uint8_t desID;
  uint8_t sourceID;
  uint8_t command;
  uint8_t data[3];
  uint8_t stopByte;
};

// Client data
typedef struct client
{
  uint8_t id;
  uint8_t type;
  uint8_t room;
  uint8_t data[3];
  uint8_t relayType[3];
  String relayName[3];
  uint8_t remoteButton[6];
  uint8_t isAlive;
};

// Remote button
typedef struct remoteButton
{
  uint8_t id;
  uint8_t relay;
  uint8_t statusOfButton;
};

#endif // __PRINTF_H__
