/**
 * Gateway Control Sample Code
 * 
 * This is a sample code use to control Gateway device to communicate with client dev
 */
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "smarthome.h"

/////////////// Define //////////////////////////////////
#define ASCII_ZERO    48
#define MAX_CLIENT    6
#define MAX_BUTTON    12
#define PIN_VT             3   
#define PIN_D0             5
#define PIN_D1             6
#define PIN_D2             7
#define PIN_D3             8

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
//RF24 radio(8,9);
RF24 radio(48,49);      // Use for atmega 2560

// Enter a MAC address
// The IP address will be provided by DHCP server:
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);


////////////// Declare global variable //////////////////
client clients[MAX_CLIENT];
uint8_t numberOfClient;
EthernetClient webConnection;
uint8_t liveFlag[MAX_CLIENT];        // The flag is used to check clients connect or disconnect with system.
uint64_t lastCheckTime;              // Last time point to check clients connect or disconnect with system.
uint8_t D0, D1, D2, D3;          // storage for data states from RF remote
remoteButton remoteButtons[MAX_BUTTON];     // Store data about buttons of remote
bool stopFlag;


////////////// Declare Function /////////////////////////
void SendCommand(uint8_t cmd, uint8_t desID, uint8_t data[3]);
void ProcessMessage(message m);
void LoadData();
void UpdateClientToUser(client updatedClient);
void ProcessWebRequest(String webRequest);
uint8_t FindClientByID(uint8_t clientID);
void SetRemoteButtons(uint8_t client, uint8_t relay, uint8_t setButtons, bool statusOfButton);

bool InitializeSD(void)
{
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(53, OUTPUT);
   digitalWrite(53, HIGH);
   pinMode(4, INPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return 0;
  }
  Serial.println("initialization done.");
  return 1;
}

void InitializeEthernet(void)
{
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(53, INPUT);

  // start the Ethernet connection and the server:
  Serial.print("Initialize Web Server...\n");
  if(Ethernet.begin(mac)==0)
  {
    Serial.print("Failed to configure Ethernet using DHCP\n");
  }
  else
  {
    server.begin();
    Serial.print("server is at \n");
    Serial.println(Ethernet.localIP());
  }  
}

void SaveData(void)
{
  uint8_t i;
  File myFile;
  
  // Switch to SD function
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);
  pinMode(4, INPUT);
  
  if(SD.exists("clients.txt"))
  {
    SD.remove("clients.txt");
  }
  
  // Create data file
  myFile = SD.open("clients.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Save data to SD card...");
    // Store numberOfClient is a first byte of data file. 
    myFile.write(numberOfClient);
    
    // Store all clients to data file
    for(i=0; i<numberOfClient; i++)
    {
      myFile.write(clients[i].id);
      myFile.write(clients[i].type);
      myFile.write(clients[i].room);
      myFile.write(clients[i].isAlive);
      myFile.write(clients[i].data[0]);
      myFile.write(clients[i].data[1]);
      myFile.write(clients[i].data[2]);
      myFile.write(clients[i].relayType[0]);
      myFile.write(clients[i].relayType[1]);
      myFile.write(clients[i].relayType[2]);    
      myFile.write(clients[i].remoteButton[0]);
      myFile.write(clients[i].remoteButton[1]);
      myFile.write(clients[i].remoteButton[2]);
      myFile.write(clients[i].remoteButton[3]);
      myFile.write(clients[i].remoteButton[4]);
      myFile.write(clients[i].remoteButton[5]);
      myFile.print(clients[i].relayName[0]);
      myFile.write('&');
      myFile.print(clients[i].relayName[1]);
      myFile.write('&');
      myFile.print(clients[i].relayName[2]);
      myFile.write('&'); 
    }
    
    // close the file:
    myFile.close();
    Serial.println("done.");
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening data file.");
  }
  
  // Use Ethernet function
  InitializeEthernet();
  
}

void LoadData(void)
{
   File myFile;
  
   // Switch to SD function
   pinMode(53, OUTPUT);
   digitalWrite(53, HIGH);
   pinMode(4, INPUT);
  
  // re-open the file for reading:
  myFile = SD.open("clients.txt");
  if (myFile) {
    Serial.println("Load data of client...\n");
    
    // Get numberOfClient at first byte of data file
    numberOfClient = myFile.read();
    char tmp_char;
    uint8_t i = 0;
    uint8_t j = 0;
    clients[0].relayName[0] = "";
    clients[0].relayName[1] = "";
    clients[0].relayName[2] = "";
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    {
      // Store data to clients
      switch(j)
      {
        case 0:
          clients[i].id = myFile.read();
          break;
        case 1:
          clients[i].type = myFile.read();
          break;
        case 2:
          clients[i].room = myFile.read();
          break;
        case 3:
          clients[i].isAlive = myFile.read();
          break;
        case 4:
          clients[i].data[0] = myFile.read();
          break;
        case 5:
          clients[i].data[1] = myFile.read();
          break;
        case 6:
          clients[i].data[2] = myFile.read();
          break;
        case 7:
          clients[i].relayType[0] = myFile.read();
          break;
        case 8:
          clients[i].relayType[1] = myFile.read();
          break;
        case 9:
          clients[i].relayType[2] = myFile.read();
          break;
        case 10:
          tmp_char = myFile.read();     
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 1, tmp_char, OFF);
          }
          else
          {
            clients[i].remoteButton[0] = 0;
          }
          break;
        case 11:
          tmp_char = myFile.read(); 
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 2, tmp_char, OFF);
          }
          else
          {
            clients[i].remoteButton[1] = 0;
          }
          break;
        case 12:
          tmp_char = myFile.read(); 
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 3, tmp_char, OFF);
          }
          else
          {
            clients[i].remoteButton[2] = 0;
          }
          break;
        case 13:
          tmp_char = myFile.read(); 
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 1, tmp_char, ON);
          }
          else
          {
            clients[i].remoteButton[3] = 0;
          }
          break;
        case 14:
          tmp_char = myFile.read(); 
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 2, tmp_char, ON);
          }
          else
          {
            clients[i].remoteButton[4] = 0;
          }
          break;
        case 15:
          tmp_char = myFile.read(); 
          if(tmp_char>0)
          {
            // Store this item to database of remote
            SetRemoteButtons(i, 3, tmp_char, ON);
          }
          else
          {
            clients[i].remoteButton[5] = 0;
          }
          break;          
        default:
          break;    
      }      
      // Read string relayNames
      if(j<=15)
      {
        j++;
      }
      else
      {
        tmp_char = myFile.read();
        if(tmp_char!='&')
        {
          clients[i].relayName[j-16] += tmp_char; 
        }
        else
        {
          if(j<18)
          {
            // Move to next relayName
            j++;
          }
          else
          {
            // Read all move to next clients
            i++;
            j=0;
            clients[i].relayName[0] = "";
            clients[i].relayName[1] = "";
            clients[i].relayName[2] = "";
          }          
        }
      }           
    }
    
    if(i!=numberOfClient)
    {
      Serial.println("Data file is worng format.\n");
      numberOfClient = i;
    }    
    // close the file:
    myFile.close();
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("Can't open data file\n");
  }  
  
  // Use Ethernet function
  InitializeEthernet(); 
}


//*************************
// Description: Use to process remote signal from User
void SetRemoteButtons(uint8_t client, uint8_t relay, uint8_t setButtons, bool statusOfButton)
{
  // Request is clear remote control for this device
  if(!setButtons)
  {
    // Clear old remoteButton control this device
    if(clients[client].remoteButton[statusOfButton*3 + relay - 1])
    {
      remoteButtons[clients[client].remoteButton[statusOfButton*3 + relay - 1] - 1].id = 0;
      remoteButtons[clients[client].remoteButton[statusOfButton*3 + relay - 1] - 1].relay = 0;
    }
    
    // This device is uncontrol by any remote button.
    clients[client].remoteButton[statusOfButton*3 + relay - 1] = 0;  
  }
  // Request is set remote control for this device.
  else
  {
    // RemoteButton is unchanged.
    if(clients[client].remoteButton[statusOfButton*3 + relay - 1] == setButtons)
    {
      // Do nothing
    }
    // RemoteButton is availabe
    else if((remoteButtons[setButtons - 1].id == 0) && (remoteButtons[setButtons - 1].relay == 0))
    {
      // Clear old remoteButton control this device
      if(clients[client].remoteButton[relay-1])
      {
        remoteButtons[clients[client].remoteButton[statusOfButton*3 + relay - 1] - 1].id = 0;
        remoteButtons[clients[client].remoteButton[statusOfButton*3 + relay - 1] - 1].relay = 0;
      }
      
      // Set this remoteButton control device
      remoteButtons[setButtons - 1].id = clients[client].id;
      remoteButtons[setButtons - 1].relay = relay;
      remoteButtons[setButtons - 1].statusOfButton = statusOfButton;
      clients[client].remoteButton[statusOfButton*3 + relay - 1] = setButtons;
    }
    else     // RemoteButton is already used for another device
    {
      // Do nothing
    }    
  } 
}

//*************************
// Description: Use to find client on database that has id item is clientID
uint8_t FindClientByID(uint8_t clientID)
{
  uint8_t i;
  for(i=0; i<numberOfClient; i++)
  {
    if(clients[i].id == clientID)
    {
      break;
    }
  }
  return i;  
}

//*************************
// Description: Use to build data for send packet
void UpdateClientToUser(client updatedClient)
{   
    //Send id
//    Serial.print(F("{\"id\":"));
//    Serial.print(updatedClient.id);
//    Serial.print(F(","));
    webConnection.print(F("{\"id\":"));
    webConnection.print(updatedClient.id);
    webConnection.print(F(","));
    
    //Send room
    webConnection.print(F("\"room\":"));
    webConnection.print(updatedClient.room);
    webConnection.print(F(","));
//    Serial.print(F("\"room\":"));
//    Serial.print(updatedClient.room);
//    Serial.print(F(","));

    //Send type
    webConnection.print(F("\"type\":"));
    webConnection.print(updatedClient.type);
    webConnection.print(F(","));
//    Serial.print(F("\"type\":"));
//    Serial.print(updatedClient.type);
//    Serial.print(F(","));

    //Send isAlive
    webConnection.print(F("\"isAlive\":"));
    webConnection.print(updatedClient.isAlive);
    webConnection.print(F(","));
//    Serial.print("\"isAlive\":");
//    Serial.print(updatedClient.isAlive);
//    Serial.print(",");

    //Send data1
    webConnection.print(F("\"data1\":"));
    webConnection.print(updatedClient.data[0]);
    webConnection.print(F(","));
//    Serial.print(F("\"data1\":"));
//    Serial.print(updatedClient.data[0]);
//    Serial.print(F(","));
    
    //Send data2
    webConnection.print(F("\"data2\":"));
    webConnection.print(updatedClient.data[1]);
    webConnection.print(F(","));
//    Serial.print(F("\"data2\":"));
//    Serial.print(updatedClient.data[1]);
//    Serial.print(F(","));
    
    //Send data3
    webConnection.print(F("\"data3\":"));
    webConnection.print(updatedClient.data[2]);
    webConnection.print(F(","));
//    Serial.print(F("\"data3\":"));
//    Serial.print(updatedClient.data[2]);
//    Serial.print(F(","));
    
    //Send relayType1
    webConnection.print(F("\"relayType1\":"));
    webConnection.print(updatedClient.relayType[0]);
    webConnection.print(F(","));
//    Serial.print("\"relayType1\":");
//    Serial.print(updatedClient.relayType[0]);
//    Serial.print(",");
    
    //Send relayType2
    webConnection.print(F("\"relayType2\":"));
    webConnection.print(updatedClient.relayType[1]);
    webConnection.print(F(","));
//    Serial.print("\"relayType2\":");
//    Serial.print(updatedClient.relayType[1]);
//    Serial.print(",");
    
    //Send relayType3
    webConnection.print(F("\"relayType3\":"));
    webConnection.print(updatedClient.relayType[2]);
    webConnection.print(F(","));
//    Serial.print("\"relayType3\":");
//    Serial.print(updatedClient.relayType[2]);
//    Serial.print(",");
    
    //Send remoteButton 1 ON
    webConnection.print(F("\"remoteButton11\":\""));
    webConnection.print(updatedClient.remoteButton[3]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton1\":\"");
//    Serial.print(updatedClient.remoteButton[0]);
//    Serial.print("\",");
    
    //Send remoteButton 1 OFF
    webConnection.print(F("\"remoteButton12\":\""));
    webConnection.print(updatedClient.remoteButton[0]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton2\":\"");
//    Serial.print(updatedClient.remoteButton[1]);
//    Serial.print("\",");
   
    //Send remoteButton 2 ON
    webConnection.print(F("\"remoteButton21\":\""));
    webConnection.print(updatedClient.remoteButton[4]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton1\":\"");
//    Serial.print(updatedClient.remoteButton[0]);
//    Serial.print("\",");
    
    //Send remoteButton 2 OFF
    webConnection.print(F("\"remoteButton22\":\""));
    webConnection.print(updatedClient.remoteButton[1]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton2\":\"");
//    Serial.print(updatedClient.remoteButton[1]);
//    Serial.print("\",");

    //Send remoteButton 3 ON
    webConnection.print(F("\"remoteButton31\":\""));
    webConnection.print(updatedClient.remoteButton[5]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton1\":\"");
//    Serial.print(updatedClient.remoteButton[0]);
//    Serial.print("\",");
    
    //Send remoteButton 3 OFF
    webConnection.print(F("\"remoteButton32\":\""));
    webConnection.print(updatedClient.remoteButton[2]);
    webConnection.print(F("\","));
//    Serial.print("\"remoteButton2\":\"");
//    Serial.print(updatedClient.remoteButton[1]);
//    Serial.print("\",");
       
    //Send relay_name1
    webConnection.print(F("\"relayName1\":\""));
    webConnection.print(updatedClient.relayName[0]);
    webConnection.print(F("\","));
//    Serial.print("\"relayName1\":\"");
//    Serial.print(updatedClient.relayName[0]);
//    Serial.print("\",");
    
    //Send relay_name2
    webConnection.print(F("\"relayName2\":\""));
    webConnection.print(updatedClient.relayName[1]);
    webConnection.print(F("\","));
//    Serial.print("\"relayName2\":\"");
//    Serial.print(updatedClient.relayName[1]);
//    Serial.print("\",");
    
    //Send relay_name3
    webConnection.print(F("\"relayName3\":\""));
    webConnection.print(updatedClient.relayName[2]);
    webConnection.print(F("\"}"));
//    Serial.print("\"relayName3\":\"");
//    Serial.print(updatedClient.relayName[2]);
//    Serial.print("\"}");     
}

//*************************
// Description: Use to receive and respond Web request
void ProcessWebRequest(String webRequest) 
{
  uint8_t i;  
  bool saveFlag = false;
  // Process data   
  if(webRequest.indexOf("GetUpdate") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("GetUpdate"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("GetUpdate"));
//    Serial.print(F("({\"client\":["));

    //Send data of clients to User
    for(i=0;i<numberOfClient;i++)
    {
      UpdateClientToUser(clients[i]); 
//      Serial.print(',');
      webConnection.print(',');
    }
  }    

  uint8_t tmpId;          // Id of target client 
  uint8_t tmpRelay;       // Position of relay of this client 
  uint8_t tmpStatus;      // New status for this relay
  uint8_t tmpCmd = 0;     // Store type of command
  if(webRequest.indexOf("SetStatusSW") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("SetStatusSW"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("SetStatusSW"));
//    Serial.print(F("({\"client\":["));
    
    // Get id
    tmpId = (webRequest.substring(webRequest.indexOf("&id=") + 4,webRequest.indexOf("&relay="))).toInt();
    
    // Get relay position
    tmpRelay = (webRequest.substring(webRequest.indexOf("&relay=") + 7,webRequest.indexOf("&status="))).toInt();     
    if((tmpRelay>=0)&&(tmpRelay<=3))
    {
      // Get status
      tmpStatus = (webRequest.substring(webRequest.indexOf("&status=") + 8,webRequest.indexOf("&_="))).toInt();   
      if((tmpStatus==ON)||(tmpStatus==OFF))
      {
        // Find this SwitchBoard
        i = FindClientByID(tmpId);
        if((i<numberOfClient) && (clients[i].type==device_switchboard))
        {         
          clients[i].data[tmpRelay-1] = tmpStatus;
          // Respond changed client to User
          UpdateClientToUser(clients[i]);         
          tmpCmd = cmd_set_status;    

          // Save data of clients to SD card
          saveFlag = true;         
        } 
      }
    }
  }
  
  if(webRequest.indexOf("DeleteClient") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("DeleteClient"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("DeleteClient"));
//    Serial.print(F("({\"client\":["));
    
    // Get id
    tmpId = (webRequest.substring(webRequest.indexOf("&id=") + 4,webRequest.indexOf("&_="))).toInt();
    
    // Find this client
    i = FindClientByID(tmpId);
    if(i<numberOfClient)
    {
      // Delete this client
      for(;i<numberOfClient-1;i++)
      {
        clients[i] = clients[i+1];
      }
      numberOfClient--;
      
      // Save data of clients to SD card
      saveFlag = true;
      
    }        
  }
  uint8_t tmpRelayType;
  String tmpRelayName;
  uint8_t tmpRemoteOnButton;
  uint8_t tmpRemoteOffButton;
  if(webRequest.indexOf("SetTypeSW") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("SetTypeSW"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("SetTypeSW"));
//    Serial.print(F("({\"client\":["));
    
    // Get id
    tmpId = (webRequest.substring(webRequest.indexOf("&id=") + 4,webRequest.indexOf("&relay="))).toInt();    
    
    // Get relay position
    tmpRelay = (webRequest.substring(webRequest.indexOf("&relay=") + 7,webRequest.indexOf("&relayType="))).toInt(); 
    
    //Get relay type
    tmpRelayType = (webRequest.substring(webRequest.indexOf("&relayType=") + 11,webRequest.indexOf("&relayName="))).toInt(); 
    
    // Get relay name
    tmpRelayName = webRequest.substring(webRequest.indexOf("&relayName=") + 11,webRequest.indexOf("&onButton")); 
    
    // Get remoteOnButton of relay
    tmpRemoteOnButton = (webRequest.substring(webRequest.indexOf("&onButton=") + 10,webRequest.indexOf("&offButton="))).toInt(); 
    
    // Get remoteOffButton of relay
    tmpRemoteOffButton = (webRequest.substring(webRequest.indexOf("&offButton=") + 11,webRequest.indexOf("&_="))).toInt();
    
    i = FindClientByID(tmpId);
    if(i<numberOfClient)
    {
      // Update to client database
      clients[i].relayType[tmpRelay - 1] = tmpRelayType;
      clients[i].relayName[tmpRelay - 1] = tmpRelayName;
      
      if((tmpRemoteOnButton != tmpRemoteOffButton) || (tmpRemoteOnButton == 0))
      {
        // Update tmpRemoteOnButton
        SetRemoteButtons(i, tmpRelay, tmpRemoteOnButton, ON);
        
        // Update tmpRemoteOnButton
        SetRemoteButtons(i, tmpRelay, tmpRemoteOffButton, OFF); 
        
        // Save data of clients to SD card
        saveFlag = true;
      }
      // Respond changed client to User
      UpdateClientToUser(clients[i]);   
    }
  }
  
  if(webRequest.indexOf("SetStatusCR") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("SetStatusCR"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("SetStatusSW"));
//    Serial.print(F("({\"client\":["));
    
    // Get id
    tmpId = (webRequest.substring(webRequest.indexOf("&id=") + 4,webRequest.indexOf("&status="))).toInt();
    
    // Get status
    tmpStatus = (webRequest.substring(webRequest.indexOf("&status=") + 8,webRequest.indexOf("&_="))).toInt();   
    if((tmpStatus==ON)||(tmpStatus==OFF))
    {
      // Find this SwitchBoard
      i = FindClientByID(tmpId);
      if((i<numberOfClient) && (clients[i].type==device_curtain))
      {         
        clients[i].data[0] = tmpStatus;        
        tmpCmd = cmd_set_status;              
        
        // Respond changed client to User
        UpdateClientToUser(clients[i]);
        
        // Save data of clients to SD card
        saveFlag = true;
      } 
    }
  }
  
  if(webRequest.indexOf("SetTypeCR") != -1)
  {
    //Send Callbak Function to User
    webConnection.print(F("SetTypeCR"));
    webConnection.print(F("({\"client\":["));
//    Serial.print(F("SetTypeSW"));
//    Serial.print(F("({\"client\":["));
    
    // Get id
    tmpId = (webRequest.substring(webRequest.indexOf("&id=") + 4,webRequest.indexOf("&name="))).toInt();    
       
    // Get relay name
    tmpRelayName = webRequest.substring(webRequest.indexOf("&name=") + 6,webRequest.indexOf("&onButton=")); 
    
    // Get remoteOnButton of relay
    tmpRemoteOnButton = (webRequest.substring(webRequest.indexOf("&onButton=") + 10,webRequest.indexOf("&offButton="))).toInt(); 
    
    // Get remoteOffButton of relay
    tmpRemoteOffButton = (webRequest.substring(webRequest.indexOf("&offButton=") + 11,webRequest.indexOf("&_="))).toInt();
    
    i = FindClientByID(tmpId);
    if(i<numberOfClient)
    {
      // Update to client database
      clients[i].relayName[0] = tmpRelayName;
      if((tmpRemoteOnButton != tmpRemoteOffButton) || (tmpRemoteOnButton == 0))
      {
        // Update tmpRemoteOnButton
        SetRemoteButtons(i, 1, tmpRemoteOnButton, ON);
        
        // Update tmpRemoteOnButton
        SetRemoteButtons(i, 1, tmpRemoteOffButton, OFF);     
        
        // Save data of clients to SD card
        saveFlag = true;
      }    
      // Respond changed client to User
      UpdateClientToUser(clients[i]); 
    }
  }  
    
  // Send end packet
  webConnection.println("]});");
  //  Serial.println("]});");
  
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  webConnection.stop();
  Serial.println("client disconnected");
  
  // Save data of clients to SD card
  if(saveFlag)
  {
    SaveData();
  }
  
  // Send User request to client device
  if(tmpCmd == cmd_set_status)
  {
    SendCommand(cmd_set_status, clients[i].id, clients[i].data);
  }      
}


//**** SendCommand Function ****
// This function use to send command
void SendCommand(uint8_t cmd, uint8_t desID, uint8_t data[3])
{
  message m;
  
  // First, stop listening so we can talk.
  radio.stopListening();
  
  // Build up message packet
  m.startByte = START_BYTE;
  m.stopByte = STOP_BYTE;
  m.sourceID = GATEWAY_ID;
  m.desID = desID;
  m.command = cmd;
  m.data[0] = data[0];
  m.data[1] = data[1];
  m.data[2] = data[2];
  
  Serial.print(F("\nMessage desID: "));
  Serial.print(m.desID);
  Serial.print(F("\nMessage sourceID: "));
  Serial.print(m.sourceID);
  Serial.print(F("\nMessage command: "));
  Serial.print(m.command);
  Serial.print(F("\nMessage data 1: "));
  Serial.print(m.data[0]);
  Serial.print(F("\nMessage data 2: "));
  Serial.print(m.data[1]);
  Serial.print(F("\nMessage data 3: "));
  Serial.print(m.data[2]);
   
  // Send command
  bool result;
  result = radio.write( &m, sizeof(message) );
     
  if(result)
  {
    Serial.print(F("\nSend command success.\n"));
  }
  else
  {
    Serial.print(F("\nSend command failed.\n"));
  }
  
  // Now, resume listening so we catch the next packets.
  radio.startListening(); 
}

//**** ProcessMessage Function ****
// This function is used to analyze received message and give a compatible respond
void ProcessMessage(message m)
{
  // Check target of this message
  if(m.desID != GATEWAY_ID)
  {
    // This message isn't sent to Gateway
    return;
  }
  uint8_t i;
  // Determine command
  switch(m.command)
  {
    case cmd_update_status:
      // Find client data to update status
      i = FindClientByID(m.sourceID);
      if(i<numberOfClient)
      {
        // Update status to client data
        clients[i].data[0] = m.data[0];
        clients[i].data[1] = m.data[1];
        clients[i].data[2] = m.data[2];
        clients[i].isAlive = true;
        liveFlag[i] = true;
        
        // Show result
        Serial.print(F("\nUpdate to client data has id: "));
        Serial.print(clients[i].id);
        Serial.print(F("\n- data 1: "));
        Serial.print(clients[i].data[0]);
        Serial.print(F("\n- data 2: "));
        Serial.print(clients[i].data[1]);
        Serial.print(F("\n- data 3: "));
        Serial.print(clients[i].data[2]);
      }
      break;
    case cmd_register :
      uint8_t i;   
      // When Gateway receives register command, Gateway checks source ID in database
      i = FindClientByID(m.sourceID);
      // If source ID is not existed in Gateway's database, Gateway will add this client to database 
      if(i == numberOfClient)
      {    
        // Update imformation of new client
        clients[numberOfClient].id = m.sourceID;
        clients[numberOfClient].type = m.data[0]; 
        clients[numberOfClient].room = m.data[1];
        clients[numberOfClient].data[0] = OFF;
        clients[numberOfClient].data[1] = OFF;  
        clients[numberOfClient].data[2] = OFF;
        clients[numberOfClient].relayType[0] = sw_undefined;
        clients[numberOfClient].relayType[1] = sw_undefined;  
        clients[numberOfClient].relayType[2] = sw_undefined;
        clients[numberOfClient].isAlive = true;
        liveFlag[numberOfClient] = true;
      
        // Show result
        Serial.print(F("\nReceive register signal form client : "));
        Serial.print(clients[numberOfClient].id);
        Serial.print(F("\nClient type : "));
        Serial.print(clients[numberOfClient].type);
        Serial.print(F("\nClient room : "));
        Serial.print(clients[numberOfClient].room);
        
        numberOfClient ++;
        
        // Save data to SD card
        SaveData();
      }
      // If source ID is existed in Gateway's database, Gateway will ignore register command
      else
      {
        break;
      }
      break;   
    default:
      break;
  }  
}

//**** setup Function ****
// This function is used to set up RF24L01 peripheral, serial peripheral and inital data of clients
void setup(void)
{
  uint8_t i;
  
  Serial.begin(57600);
  printf_begin();

  // Initial Remote Function
  // setup the R06A decoder connections
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  pinMode(PIN_D2, INPUT);
  pinMode(PIN_D3, INPUT);
  pinMode(PIN_VT, INPUT); 
  stopFlag = false; 
  // Inital data of buttons
  for(i=0; i<MAX_BUTTON; i++)
  {
    remoteButtons[i].id = 0;
    remoteButtons[i].relay = 0;  
  }

  // Load data of client  
  InitializeSD();
  LoadData(); 

  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(sizeof(struct message));

  // Open pipes to other nodes for communication
  radio.openWritingPipe(CLIENT_PIPE);
  radio.openReadingPipe(1,GATEWAY_PIPE);
     
  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails(); 
    
  // Send request status command to all clients
  SendCommand(cmd_update_status,BROADCAST_ID,0);

  // Initial Health Check cycle
  // At starting, all clients are "death"
  for(i=0; i<numberOfClient; i++)
  {
    liveFlag[i] = false;
  }
  // Store current time
  lastCheckTime = millis();  
  

}

//**** loop Function ****
// This function is used to get request from user and receive a message from client
void loop(void)
{
  uint8_t inputData[10];
  uint8_t i;
  bool inputFlag = false;
  message m;
  //////////////////////////////////////////////////////////  
  // Get request from User Browser
  //////////////////////////////////////////////////////////
  //Check input string
  String webRequest = "";
  // listen for incoming clients
  webConnection = server.available();
  if (webConnection) 
  {
    Serial.println(F("new client"));

    // Check and read data sent from client
    i=0;
    while(webConnection.available())
    {
      char c = webConnection.read();
      if(!inputFlag)
      {
        webRequest += c;
      }
      if(c == '\n')
      {
        inputFlag = true;
      }
    }    
    Serial.print(webRequest);
    ProcessWebRequest(webRequest); 
  } 
  
  
  //////////////////////////////////////////////////////////  
  // Get request from User with Serial Interface
  //////////////////////////////////////////////////////////
  //Check input string
  i = 0;
  inputFlag = false;
  while (Serial.available()) 
  {
    // get the new byte:
    inputData[i] = (char)Serial.read(); 
    i++;
    inputFlag = true;
    delay(20);
  }
  
  // Process input string
  if(inputFlag)
  {
    inputData[i] = '\0';
    Serial.print(F("\nYour Input: \n"));
    printf("%s",inputData);
    
    // Request show list of client
    if(inputData[0] == 'f')
    {  
      Serial.print(F("\n***** List of Clients ****"));
      for(i=0; i<numberOfClient; i++)
      {
        Serial.print(F("\n* Client "));
        Serial.print(i + 1);
        Serial.print(F("\n  - Client ID: "));
        Serial.print(clients[i].id);
        Serial.print(F("\n  - Client type: ")); 
        Serial.print(clients[i].type);
        Serial.print(F("\n  - Client data 1: "));
        Serial.print(clients[i].data[0]);
        Serial.print(F("\n  - Client data 2: "));
        Serial.print(clients[i].data[1]);
        Serial.print(F("\n  - Client data 3: "));
        Serial.print(clients[i].data[2]);
        Serial.print(F("\n  - Client live status: "));
        Serial.print(clients[i].isAlive);              
      }
    } 
    
    // Request save data to SD card
    if(inputData[0] == 's')
    {
      SaveData();
    }
    
    // Request load data from SD card
    if(inputData[0] == 'l')
    {
      LoadData();
    }
    
    //Request send command
    else     
    {
      /*
      Command pattern: cmd desID switchButton status
        cmd: command used to send to client
        desID: ID of target client
        switchButton: Button on switchBoard need to change status (use for set new status command)
        status: New status of button
      */
      uint8_t cmd;
      uint8_t desID;
      uint8_t switchButton;
      uint8_t data;
     
      // Get information from request
      cmd = inputData[0] - ASCII_ZERO;       //Convert to number
      desID = inputData[1] - ASCII_ZERO;     //Convert to number
      for(i=0; i<numberOfClient; i++)
      {
        if(clients[i].id == desID)
        {
          switch(cmd)
          {
            case cmd_set_status:
              // Find the client to set status  
              switchButton = inputData[2] - ASCII_ZERO;    //Convert to number
              data = inputData[3] - ASCII_ZERO;
              // Check input data is invalid or not
              if(((data == ON) || (data == OFF)) && (switchButton>=1) && (switchButton<=3))
              {
                // Update new status to client data
                clients[i].data[switchButton-1] = data;
                // Call send command
                SendCommand(cmd_set_status, clients[i].id, clients[i].data);
              }
              break;
            case cmd_request_status:
              // Call send command with data field is unnescessary
              SendCommand(cmd_request_status, clients[i].id, clients[i].data);
              break;
            default:
              break;
          }
        }         
      }
    }
  }
  
  //////////////////////////////////////////////////////////////////
  // Get message from client
  //////////////////////////////////////////////////////////////////
  if(radio.available())
  {
    // Read message
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &m, sizeof(struct message) );  
    }
  
    Serial.print(F("\n\nGet message:"));      
    Serial.print(F("\nMessage desID: "));
    Serial.print(m.desID);
    Serial.print(F("\nMessage sourceID: "));
    Serial.print(m.sourceID);
    Serial.print(F("\nMessage command: "));
    Serial.print(m.command);
    Serial.print(F("\nMessage data 1: "));
    Serial.print(m.data[0]);
    Serial.print(F("\nMessage data 2: "));
    Serial.print(m.data[1]);
    Serial.print(F("\nMessage data 3: "));
    Serial.print(m.data[2]);
  
    // Analyze message
    ProcessMessage(m);
  } 
    
  //////////////////////////////////////////////////////////////////
  // Health Check Cycle
  //////////////////////////////////////////////////////////////////
  if(lastCheckTime > millis())                      // Timer of Arduino resets after 50 days
  {
    lastCheckTime = 0;
  }
  
  else if((millis() - lastCheckTime) > 300000)      // Checking after 5 minutes ~ 300000 miliseconds
  {
    for(i=0; i<numberOfClient; i++)
    {
      if(!liveFlag[i])
      {
        clients[i].isAlive = false;
      }
      else
      {
        clients[i].isAlive = true;
        liveFlag[i] = false;          // Reset liveFlag
      }
      lastCheckTime = millis();                      // Store current time          
    }
  }  
  
  //////////////////////////////////////////////////////////////////
  // Remote Function
  //////////////////////////////////////////////////////////////////
  // Read status pin VT of receive module
  uint8_t tmpRemoteButton;
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
    tmpRemoteButton = D0 | (D1 << 1) | (D2 << 2) | (D3 << 3);
    Serial.print("\nPress Button:");
    Serial.print(tmpRemoteButton);
    if((tmpRemoteButton > 0) && (tmpRemoteButton < 13))
    {
      if((remoteButtons[tmpRemoteButton - 1].id !=0) && (remoteButtons[tmpRemoteButton - 1].relay !=0))
      {
        i = FindClientByID(remoteButtons[tmpRemoteButton - 1].id);
        if(i < numberOfClient)
        {
          clients[i].data[remoteButtons[tmpRemoteButton - 1].relay - 1] = remoteButtons[tmpRemoteButton - 1].statusOfButton;
          SendCommand(cmd_set_status, clients[i].id, clients[i].data);
        }
      }
    }
  }
  else if (!digitalRead(PIN_VT) && stopFlag)
  {
    stopFlag = false;     
  }  
}

