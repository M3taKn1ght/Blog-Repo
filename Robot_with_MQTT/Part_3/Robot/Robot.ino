//-----------------------------------------------------
// ESP-NodeMCU robot
// mqtt-broker and mapping analog input
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Jan 2021
// Update:  29. Jan 2021
//-----------------------------------------------------
#include <WiFi.h>
#include <PubSubClient.h> //Lib for MQTT Pub and Sub

//Define WiFi-Settings
#ifndef STASSID
#define STASSID ""       //Enter Wfi-Name
#define STAPSK  ""  //Enter Passkey
#endif

#define ADVANCEDIAG 1

#define MAXSPEED 255
#define MINSPEED 155
//MQTT stuff
const char* MQTT_BROKER = "Pi4";  //Name of the mqtt broker
String clientID = "AZBot";  //Clientname for MQTT-Broker
const char* SubTopicStraight = "/RemoteControl/Straight"; //Topic first temp
const char* SubTopicCross = "/RemoteControl/Cross";       //Topic second temp

int iMQTTStraight, iMQTTCross, iMQTTStraightNew, iMQTTCrossNew, iMQTTStraightLast, iMQTTCrossLast;

//Create objects for mqtt
WiFiClient espClient;
PubSubClient mqttClient(espClient);

//Timer-vars for debounce
unsigned long ulDebounce = 10;  //Debounce-time
unsigned long ulLastDebounceTimeStraight, ulLastDebounceTimeCross;      //Timer to debouce button

//PWM and motor configuration
// Motor A
const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int enable1Pin = 14;
const int motor1channel = 0;
// Motor B
const int motor2Pin1 = 17;
const int motor2Pin2 = 5;
const int enable2Pin = 16;
const int motor2channel = 1;

// Setting PWM properties
const int freq = 30000;
const int resolution = 8;

bool bUpdateMovement = false; //Will set, if there are new movements from mqtt available
/*
  =================================================================
  Function:     setup
  Returns:      void
  Description:  Needed setup-function
  =================================================================
*/
void setup()
{
  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);
  Serial.begin(115200);
  Serial.println("Remote control started");
  iMQTTStraightNew = 0;
  iMQTTCrossNew = 0;
  writeAdvanceDiag("Init WiFi", true);
  setupWifi();
  writeAdvanceDiag("Init Wifi - DONE", true);
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER, 1883);
  writeAdvanceDiag("Set Callback-function", true);
  mqttClient.setCallback(callback);
  writeAdvanceDiag("Set PWM-Channels", true);
  ledcSetup(motor1channel, freq, resolution); //Configurate PWM for motor 1
  ledcSetup(motor2channel, freq, resolution); //Configurate PWM for motor 2
  ledcAttachPin(enable1Pin, motor1channel); //Attach channel 1 to motor 1
  ledcAttachPin(enable2Pin, motor2channel); //Attach channel 2 to motor 2

  writeAdvanceDiag("Finish setup()-Function", true);
}

/*
  =================================================================
  Function:     loop
  Returns:      void
  Description:  Needed loop-function
  =================================================================
*/
void loop()
{
  if (!mqttClient.connected())
    reconnectMQTT();

  mqttClient.loop();
  DebounceStraight();
  DebounceCross();
  int iSpeedMotor1, iSpeedMotor2;
  if (bUpdateMovement)  //Check if there is a new movement available from mqtt
  {
    Serial.println("Current value straight: " + String(iMQTTStraight));
    Serial.println("Current value cross: " + String(iMQTTCross));
    if (iMQTTStraight != 0)
    {
      if (iMQTTStraight < 0)
      {
        digitalWrite(motor1Pin1, LOW);
        digitalWrite(motor1Pin2, HIGH);
        digitalWrite(motor2Pin1, LOW);
        digitalWrite(motor2Pin2, HIGH);
      }
      else
      {
        digitalWrite(motor1Pin1, HIGH);
        digitalWrite(motor1Pin2, LOW);
        digitalWrite(motor2Pin1, HIGH);
        digitalWrite(motor2Pin2, LOW);
      }
      if(abs(iMQTTStraight) == 1)
      {
        iSpeedMotor1 = MAXSPEED - (MAXSPEED - MINSPEED)/2;
        iSpeedMotor2 = MAXSPEED - (MAXSPEED - MINSPEED)/2;
      }
      else
      {
        iSpeedMotor1 = MAXSPEED;
        iSpeedMotor2 = MAXSPEED;
      }
    }
    else
    {
      iSpeedMotor1 = 0;
      iSpeedMotor2 = 0;
    }

    if (iMQTTCross != 0)
    {
      if (iMQTTCross < 0)
      {
        if(iSpeedMotor1 == MAXSPEED)
        {
          if(abs(iMQTTCross) == 1)
            iSpeedMotor1 = MAXSPEED - (MAXSPEED - MINSPEED)/2;
          else
            iSpeedMotor1 = MINSPEED;
        }
        else
        {
          if(abs(iMQTTCross) == 1)
            iSpeedMotor1 = MINSPEED;
          else
            iSpeedMotor1 = 0;
        }
        Serial.println("New Speed motor 1: " + String(iSpeedMotor1));
      }
      else
      {
        if(iSpeedMotor2 == MAXSPEED)
        {
          if(abs(iMQTTCross) == 1)
            iSpeedMotor2 = MAXSPEED - (MAXSPEED - MINSPEED)/2;
          else
            iSpeedMotor2 = MINSPEED;
        }
        else
        {
          if(abs(iMQTTCross) == 1)
            iSpeedMotor2 = MINSPEED;
          else
            iSpeedMotor2 = 0;
        }
        Serial.println("New Speed motor 2: " + String(iSpeedMotor2));
      }
    }
    //Write speed to motor pwm
    ledcWrite(motor1channel, iSpeedMotor1);
    ledcWrite(motor2channel, iSpeedMotor2);
    bUpdateMovement = false;  //New movement set
  }

}

/*
  =================================================================
  Function:     setupWifi
  Returns:      void
  Description:  Setup wifi to connect to network
  =================================================================
*/
void setupWifi()
{
  Serial.println("Connection to: " + String(STASSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/*
  =================================================================
  Function:     callback
  Returns:      void
  Description:  Will automatical called, if a subscribed topic
                has a new message
  topic:        Returns the topic, from where a new msg comes from
  payload:      The message from the topic
  length:       Length of the msg, important to get conntent
  =================================================================
*/
void callback(char* topic, byte* payload, unsigned int length)
{
  String strMessage = "";
  writeAdvanceDiag("Message arrived from topic: " + String(topic), true);
  writeAdvanceDiag("Message length: " + String(length), true);
  for (int i = 0; i < length; i++)
    strMessage += String((char)payload[i]);
  writeAdvanceDiag("Message is: " + strMessage, true);
  if (String(topic) == String(SubTopicStraight))
  {
    iMQTTStraightNew = strMessage.toInt();
  }
  else if (String(topic) == String(SubTopicCross))
  {
    iMQTTCrossNew = strMessage.toInt();
  }
}

/*
  =================================================================
  Function:     reconnectMQTT
  Returns:      void
  Description:  If there is no connection to MQTT, this function is
                called. In addition, the desired topic is registered.
  =================================================================
*/
void reconnectMQTT()
{
  while (!mqttClient.connected())
  {
    writeAdvanceDiag("Login to MQTT-Broker", true);
    if (mqttClient.connect(clientID.c_str()))
    {
      Serial.println("Connected to MQTT-Broker " + String(MQTT_BROKER));
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicStraight) + "'", true);
      mqttClient.subscribe(SubTopicStraight, 1); //Subscibe topic "SubTopicStraight"
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicCross) + "'", true);
      mqttClient.subscribe(SubTopicCross, 1); //Subscibe topic "SubTopicCross"
    }
    else
    {
      writeAdvanceDiag("Failed with rc=" + String(mqttClient.state()), true);
      Serial.println("Next MQTT-Connect in 3 sec");
      delay(3000);
    }
  }
}


/*
  =================================================================
  Function:     writeAdvanceDiag
  Returns:      void
  Description:  Writes advance msg to serial monitor, if
                ADVANCEDIAG >= 1
  msg:          Message for the serial monitor
  newLine:      Message with linebreak (true)
  =================================================================
*/
void writeAdvanceDiag(String msg, bool newLine)
{
  if (bool(ADVANCEDIAG)) //Check if advance diag is enabled
  {
    if (newLine)
      Serial.println(msg);
    else
      Serial.print(msg);
  }
}

/*
  =================================================================
  Function:     DebounceStraight
  Returns:      void
  Description:  Set new value, if debouce is over
                If there is a new valid bUpdateMovement 
                will set true
  =================================================================
*/
void DebounceStraight()
{
  if (iMQTTStraightNew != iMQTTStraightLast)
    ulLastDebounceTimeStraight = millis();

  if ((millis() - ulLastDebounceTimeStraight) > ulDebounce)
  {

    if (iMQTTStraightNew != iMQTTStraight)
    {
      iMQTTStraight = iMQTTStraightNew;
      writeAdvanceDiag("New straight value " + String(iMQTTStraight), true);
      bUpdateMovement = true;
    }
  }
  iMQTTStraightLast = iMQTTStraightNew;
  
}

/*
  =================================================================
  Function:     DebounceCross
  Returns:      void
  Description:  Set new value, if debouce is over
                If there is a new valid bUpdateMovement 
                will set true
  =================================================================
*/
void DebounceCross()
{
  if (iMQTTCrossNew != iMQTTCrossLast)
    ulLastDebounceTimeCross = millis();

  if ((millis() - ulLastDebounceTimeCross) > ulDebounce)
  {
    if (iMQTTCrossNew != iMQTTCross)
    {
      iMQTTCross = iMQTTCrossNew;
      writeAdvanceDiag("New cross value " + String(iMQTTCross), true);
      bUpdateMovement = true;
    }
  }
  iMQTTCrossLast = iMQTTCrossNew;
}
