#! /usr/bin/env python

import serial
import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

#poweron the RPI GSM SIM900 module
def PowerON_SIM900_module():
    GPIO.setup(22,GPIO.OUT)
    GPIO.output(22,GPIO.HIGH)
    return;

def SendSMS(value):
   #send SMS about the action
   ser=serial.Serial('/dev/ttyAMA0',9600,timeout=1);
   ser.open();
   ser.write("at\r");
   time.sleep(3);
   line=ser.read(size=64);
   print line;
   ser.write('AT+CMGS="+84988807067"\r');
   time.sleep(3);
   if (value==1):
       ser.write('Relay powered ON!\r');
   elif (value==0):
       ser.write('Relay powered OFF!\r');
   time.sleep(3);
   ser.write(chr(26));
   ser.close();
   return;

def CheckNewUnreadMessage():
   print "Check for new messages..\n";
   ser=serial.Serial('/dev/ttyAMA0',9600,timeout=1);
   ser.open();
   ser.write("at\r");
   time.sleep(3);
   line=ser.read(size=64);
   #print line;
   ser.write('AT+CMGL="REC UNREAD"\r')
   time.sleep(3);
   response=ser.read(size=200);
   print response;
   ser.close();
   return response;

#main loop
PowerON_SIM900_module();
print "Registering the sim...\n";
time.sleep(15); #wait for sim to resgister to the net
print "Started main loop\n";
while True:
  print "Sending new message\n";
  SendSMS(1);
  print "Receiving message\n";
  CheckNewUnreadMessage();
  time.sleep(5);