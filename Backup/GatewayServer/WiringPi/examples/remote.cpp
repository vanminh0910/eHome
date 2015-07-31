
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.

#define PIN_VT             4   
#define PIN_D0             0
#define PIN_D1             1
#define PIN_D2             2
#define PIN_D3             3

bool stopFlag;

int main (void)
{
  printf ("Raspberry Pi blink\n") ;

  if (wiringPiSetup () == -1)
    return 1 ;

  // Initial Remote Function
  // setup the R06A decoder connections
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  pinMode(PIN_D2, INPUT);
  pinMode(PIN_D3, INPUT);
  pinMode(PIN_VT, INPUT); 
  stopFlag = false; 

  for (;;)
  {
    // Read status pin VT of receive module
	  int tmpRemoteButton;
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
		int D0 = digitalRead(PIN_D0);    
		int D1 = digitalRead(PIN_D1);
		int D2 = digitalRead(PIN_D2);
		int D3 = digitalRead(PIN_D3);
		stopFlag = true;
		tmpRemoteButton = D0 | (D1 << 1) | (D2 << 2) | (D3 << 3);
		printf("\nPress Button:");
		printf(tmpRemoteButton);		
	  }
	  else if (!digitalRead(PIN_VT) && stopFlag)
	  {
		stopFlag = false;     
	  }  
  }
  return 0 ;
}