// Heartbeat example 2 for master (Arduino Uno) 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 15. Sep 2020
// Update:  15. Sep 2020
//-----------------------------------------------------

#define HEARTBEAT_RECEIVE_INTERVAL 2000 //Maximum allowed time for normal change
#define HEARTBEAT_CHANGE_INTERVAL 3000  //Needed time for rebooting slave

int iCount = 0;  //Global var for monitoring
unsigned long previousMillis = 0; //Stores current millis to switch Heartbeat-Signal
int iLastHeartbeatStatus = HIGH; //Last know status from slave
int iSlavePin = 2;  //Pin to receive slave IO
int iSignalPin = 3; //Pin to set new flag
bool bSlaveStarting = true; //Bool to switch during "normal" mode and slave is (re-)starting
bool bCheckNewSignal = false;
//Init Arduino
void setup()
{
  Serial.begin(115200); // Active serial com with baudrate 115200
  Serial.println("Arduino (master) started ...");  //Say hello to the world
  pinMode(iSlavePin, INPUT); // Digital-Pin 2 as input}
  pinMode(iSignalPin, OUTPUT);  //Digital-Pin 3 as output
  pinMode(13, OUTPUT);  //Output to reset slave
  digitalWrite(iSignalPin, iLastHeartbeatStatus); //Needs per default set to HIGH
}

//Simple loop-function
void loop()
{
  delay(200); // just wait a few seconds before starting next loop
  unsigned long currentMillis = millis(); //Get current millis
  if(!bSlaveStarting) //Slave is started
  {
    if(bCheckNewSignal) //Check if master need to get status from slave
    {
      if(digitalRead(iSlavePin) == iLastHeartbeatStatus)
      {
        Serial.println("Change: " + String(iCount) + " - Receive signal " + bool(iLastHeartbeatStatus));
        bCheckNewSignal = false;
        iCount++;
      }
    }
  }
  else  //(Re-)starting mode from slave
  {
    if(digitalRead(iSlavePin) == iLastHeartbeatStatus)  //So there must be something that sends signal
    {
      Serial.println("Slave online");
      bSlaveStarting = false; //Switch to normal mode
      previousMillis = millis();  //Overwrite previousMillis
    }
  }

  //Check if Master needs to change state and write to SignalPin
  if(currentMillis - previousMillis >= int(HEARTBEAT_CHANGE_INTERVAL))
  {
    if(!bSlaveStarting)
    {
      if(iLastHeartbeatStatus == LOW)
        iLastHeartbeatStatus = HIGH;
      else
        iLastHeartbeatStatus = LOW;
      
      digitalWrite(iSignalPin,iLastHeartbeatStatus);
      Serial.println("Change master signal to: " + String(bool(iLastHeartbeatStatus)));
      bCheckNewSignal = true;
    }
    previousMillis = currentMillis;
  }
  
  //Check if reboot from slave is needed or not
  if(currentMillis - previousMillis >= int(HEARTBEAT_RECEIVE_INTERVAL) && bCheckNewSignal)
    ResetSlave(); //Reset slave and overwrite some vars

} 

//Function to reset slave
void ResetSlave()
{
  Serial.println("Set RESET-Pin slave to LOW");
  digitalWrite(13, LOW);  //Reset slave
  delay(500); //Wait 500ms, so slave restarts
  Serial.println("Set RESET-Pin slave to HIGH");
  digitalWrite(13, HIGH); //Lets start slave up
  previousMillis = millis();  //Overwrite previousMillisSlave
  bSlaveStarting = true;  //Slave is restarting so switch to (re-)start mode
  iLastHeartbeatStatus = HIGH;
  bCheckNewSignal = false;
}
