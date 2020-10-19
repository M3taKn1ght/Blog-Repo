// Heartbeat example 1 for slave (ESP32) 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 15. Sep 2020
// Update:  15. Sep 2020
//-----------------------------------------------------

#define HEARTBEAT_INTERVAL 1000

int iCount = 0;  //Global var for monitoring
unsigned long previousMillis = 0;
int iLastHeartbeatStatus = LOW;
//Init ESP32
void setup()
{
  Serial.begin(115200);     // Active serial com with baudrate 9600
  Serial.println("ESP started ...");  //Say hello to the world
  pinMode(14, OUTPUT); // Digital-Pin 0 as input}
  digitalWrite(14, LOW);  //Init as with LOW SIGNAL
}

//Simple loop-function
void loop()
{
  unsigned long currentMillis = millis(); //Get current millis
  if(currentMillis - previousMillis >= HEARTBEAT_INTERVAL)  //Check if interval is equal or over 1 second
  {
    Serial.print("Loop: "); //Write loop
    Serial.println(iCount); //Show current counter
    previousMillis = currentMillis; //Overwrite stored previous millis
    if(iLastHeartbeatStatus == LOW) //Check if last signal was HIGH or LOW
    {
      Serial.println("Set Heartbeat to HIGH");  //Some message for serial monitor
      iLastHeartbeatStatus = HIGH;  //Set signal to HIGH
    }
    else
    {
      Serial.println("Set Heartbeat to LOW"); //Some message for serial monitor
      iLastHeartbeatStatus = LOW; //Set signal to LOW
    }
    digitalWrite(14, iLastHeartbeatStatus);  // Write new status to digital pin
    iCount++; //Increase counter
  }
  delay(200); // just wait a few seconds before starting next loop
}
