// Heartbeat example 1 for master (Arduino Uno) 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 15. Sep 2020
// Update:  15. Sep 2020
//-----------------------------------------------------

#define HEARTBEAT_SHORT_INTERVAL 2000 //Maximum allowed time for normal change
#define HEARTBEAT_LONG_INTERVAL 5000  //Needed time for rebooting slave

int iCount = 0;  //Global var for monitoring
unsigned long previousMillis = 0; //Stores current millis to check reset needed
int iCurrentInterval = 0; //Var to set correct interval, see define
int iLastHeartbeatStatus = LOW; //Last know status from slave
int iSlavePin = 2;  //Pin to receive slave IO
bool bSlaveStarting = true; //Bool to switch during "normal" mode and slave is (re-)starting
//Init Arduino
void setup()
{
  Serial.begin(115200); // Active serial com with baudrate 115200
  Serial.println("Arduino (master) started ...");  //Say hello to the world
  pinMode(iSlavePin, INPUT); // Digital-Pin 0 as input}
  pinMode(13, OUTPUT);  //Output to reset slave
  digitalWrite(13, HIGH); //Needs per default set to HIGH
  iCurrentInterval = HEARTBEAT_LONG_INTERVAL; //Lets wait 5 sec during startup
}

//Simple loop-function
void loop()
{
  delay(200); // just wait a few seconds before starting next loop
  unsigned long currentMillis = millis(); //Get current millis
  if(!bSlaveStarting) //Normal mode 2 sec
  {
    //Check if status from slave change from LOW to HIGH
    if(iLastHeartbeatStatus == LOW && digitalRead(iSlavePin) == HIGH)
    {
      previousMillis = currentMillis;
      iLastHeartbeatStatus = HIGH;
      Serial.print("Change: "); //Write loop
      Serial.print(iCount); //Show current counter
      Serial.println(" - Got heartbeat HIGH");
      iCount++;
    }

    //Check if status from slave change from HIGH to LOW
    if(iLastHeartbeatStatus == HIGH && digitalRead(iSlavePin) == LOW)
    {
      previousMillis = currentMillis;
      iLastHeartbeatStatus = LOW;
      Serial.print("Change: "); //Write loop
      Serial.print(iCount); //Show current counter
      Serial.println(" - Got heartbeat LOW");
      iCount++;
    }
  }
  else  //(Re-)starting mode 5 sec
  {
    if(digitalRead(iSlavePin) == HIGH)  //So there must be something that sends signal
    {
      Serial.println("Slave online");
      iLastHeartbeatStatus = HIGH;
      bSlaveStarting = false; //Switch to normal mode
      previousMillis = millis();  //Overwrite previousMillis
      iCurrentInterval = HEARTBEAT_SHORT_INTERVAL;  // 2 seconds wait for switching 
      iCount++;
    }
  }

  //Check if reboot from slave is needed or not
  if(currentMillis - previousMillis >= iCurrentInterval)
  {
    ResetSlave(); //Reset slave and overwrite some vars
  }  

}

//Function to reset slave
void ResetSlave()
{
  Serial.println("Set RESET-Pin slave to LOW");
  digitalWrite(13, LOW);  //Reset slave
  delay(500); //Wait 500ms, so slave restarts
  Serial.println("Set RESET-Pin slave to HIGH");
  digitalWrite(13, HIGH); //Lets start slave up
  previousMillis = millis();  //Overwrite previousMillis
  iCurrentInterval = HEARTBEAT_LONG_INTERVAL; //Saftey for waiting slave restart
  bSlaveStarting = true;  //Slave is restarting so switch to (re-)start mode
}
