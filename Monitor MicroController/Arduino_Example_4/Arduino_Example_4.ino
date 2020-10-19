// Watchdog fourth example for Arduino 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 11. Sep 2020
// Update:  11. Sep 2020
//-----------------------------------------------------

#include <avr/wdt.h> //Load Watchdog-Library
#define LOOP_COUNT 20
int iCount = 0;  //Global var for monitoring

//Init arduino
void setup()
{
  Serial.begin(9600);     // Active serial com with baudrate 9600
  Serial.println("Arduino started ...");  //Say hello to the world
  wdt_enable(WDTO_4S);   // Set watchdog timer to 4 seconds
}

//Simple loop-function
void loop()
{
  for(iCount = 0; iCount < LOOP_COUNT; iCount++)
  {
    delay(1000);
    Serial.print("Seconds: ");
    Serial.print(iCount);
    Serial.println(" - Resetting Watchdog");
    wdt_reset();
  }
  Serial.println("End of loop");
}
