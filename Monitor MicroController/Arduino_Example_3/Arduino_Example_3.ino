// Watchdog third example for Arduino 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 11. Sep 2020
// Update:  11. Sep 2020
//-----------------------------------------------------

#include <avr/wdt.h> //Load Watchdog-Library

int iCount = 0;  //Global var for monitoring

//Init arduino
void setup()
{
  pinMode(3, INPUT);      // Digital-Pin 3 as input
  digitalWrite(3, HIGH);  // Activate pullup-ressitormode
  Serial.begin(9600);     // Active serial com with baudrate 9600
  Serial.println("Arduino started ...");  //Say hello to the world
  wdt_enable(WDTO_4S);   // Set watchdog timer to 4 seconds
}

//Simple loop-function
void loop()
{
  Serial.print(iCount); //Show current counter
  Serial.println(": waiting for sensor ...");
  GetSensorData();
  wdt_reset();
  if (iCount == 10)  //After 10 cyclus disable watchdog
  {
    wdt_disable();  // Disable watchdog
    Serial.println("WD disabled ...");  //Write information to serial monitor
  }
  iCount++; //Increase counter
}

//Check current sensor data
void GetSensorData() 
{
  while (digitalRead(3) == HIGH)  //Loop till button pressed
  {
    delay(500);
  }
  Serial.println("Sensordata received ...");
  delay(1000); 
}
