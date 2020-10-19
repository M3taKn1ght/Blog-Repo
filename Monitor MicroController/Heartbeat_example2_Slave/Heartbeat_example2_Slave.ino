// Heartbeat example two for slave (ESP32) 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 15. Sep 2020
// Update:  15. Sep 2020
//-----------------------------------------------------

int iCount = 0;  //Global var for monitoring
int iLastHeartbeatStatus = LOW;
//Init ESP32
void setup()
{
  Serial.begin(115200);     // Active serial com with baudrate 9600
  Serial.println("ESP started ...");  //Say hello to the world
  pinMode(14, OUTPUT);  // Digital-Pin 14 as output
  pinMode(27, INPUT);   // Pin 27 as input
  digitalWrite(14, LOW);  //Init as with LOW SIGNAL
}

//Simple loop-function
void loop()
{
  if(digitalRead(27) != iLastHeartbeatStatus)
  {//Check if Input Pin 27 is different to iLastHeartbeatStatus
    iLastHeartbeatStatus = digitalRead(27);   //Get new iLastHeartbeatStatus
    digitalWrite(14, iLastHeartbeatStatus);    //Mirror and anser to master
    Serial.print(iCount);                     //Print to SerialMonitor
    Serial.print(" - Set Heartbeat to ");         
    Serial.println(iLastHeartbeatStatus);
    iCount++; //Increase counter
  }
  delay(200); //Short delay 
}
