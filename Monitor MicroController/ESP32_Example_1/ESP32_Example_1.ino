// Watchdog first example for ESP32 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 14. Sep 2020
// Update:  14. Sep 2020
//-----------------------------------------------------

#include <esp_task_wdt.h> //Load Watchdog-Library
#define WDT_TIMEOUT_SECONDS 3

int iCount = 0;  //Global var for monitoring
//Init ESP32
void setup()
{
  Serial.begin(115200);     // Active serial com with baudrate 9600
  Serial.println("ESP started ...");  //Say hello to the world
  pinMode(0, INPUT_PULLUP); // Digital-Pin 0 as input
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS,true);  //Init Watchdog with 3 seconds timeout and panicmode
  esp_task_wdt_add(NULL); //No special task needed
}

//Simple loop-function
void loop()
{
  Serial.print(iCount); //Show current counter
  Serial.println(": waiting for sensor ...");

  GetSensorData();
  
  esp_task_wdt_reset();
  if(iCount == 3) //After 3 cyclus disable watchdog
  {
    Serial.println("Disable watchdog"); //Write information to serial monitor
    esp_task_wdt_deinit();  // Disable watchdog
  }
  iCount++; //Increase counter
}

//Check current sensor data
void GetSensorData()
{
  while(digitalRead(0) == HIGH) //Loop till button pressed
  {
    delay(500);
  }
  Serial.println("Sensordata received ...");
  delay(1000); 
}
