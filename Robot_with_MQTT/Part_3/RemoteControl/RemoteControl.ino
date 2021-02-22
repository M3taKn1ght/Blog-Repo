//-----------------------------------------------------
// ESP-NodeMCU remotecontroller 
// mqtt-broker and mapping analog input
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Jan 2021
// Update:  20. Jan 2021
//-----------------------------------------------------
#include <WiFi.h>
#include <PubSubClient.h> //Lib for MQTT Pub and Sub

//Define WiFi-Settings
#ifndef STASSID
#define STASSID ""       //Enter Wfi-Name
#define STAPSK  ""  //Enter Passkey
#endif

#define ADVANCEDIAG 1

#define ADC_STRAIGHT 36
#define ADC_CROSS 39

const char* MQTT_BROKER = "Pi4";  //Name of the mqtt broker
const char* PubTopicStraight = "/RemoteControl/Straight"; //Topic first temp
const char* PubTopicCross = "/RemoteControl/Cross";       //Topic second temp
String clientID = "RemoteController";  //Clientname for MQTT-Broker

int iLastStraight, iLastCross;
//Create objects for mqtt
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];


void setup() 
{
  Serial.begin(115200);
  Serial.println("Remote control started");
  writeAdvanceDiag("Init WiFi", true);
  setupWifi();
  writeAdvanceDiag("Init Wifi - DONE", true);
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER,1883);
  iLastStraight = -7;
  iLastCross = -7;
  writeAdvanceDiag("Finish setup()-Function", true);
}

void loop() {
  if(!mqttClient.connected())
    reconnectMQTT();

  mqttClient.loop();
  //Read value from analog input and map value
  int iMappedStraight = map(analogRead(ADC_STRAIGHT),4095,0,-2,2);
  if(iMappedStraight != iLastStraight)
  {
    snprintf(msg,MSG_BUFFER_SIZE, "%1d",iMappedStraight);  //Convert message to char
    mqttClient.publish(PubTopicStraight,msg,true);  //Send to broker
    writeAdvanceDiag("Send Straight: " + String(iMappedStraight), true);
    iLastStraight = iMappedStraight;
  }
  //Read value from analog input and map value
  int iMappedCross = map(analogRead(ADC_CROSS),4095,0,-2,2);
  if(iMappedCross != iLastCross)
  {
    snprintf(msg,MSG_BUFFER_SIZE, "%1d",iMappedCross);  //Convert message to char
    mqttClient.publish(PubTopicCross,msg,true);  //Send to broker
    writeAdvanceDiag("Send Cross: " + String(iMappedCross), true);
    iLastCross = iMappedCross;
  }
}

/*
* =================================================================
* Function:     setupWifi   
* Returns:      void
* Description:  Setup wifi to connect to network
* =================================================================
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
* =================================================================
* Function:     reconnectMQTT   
* Returns:      void
* Description:  If there is no connection to MQTT, this function is
*               called. In addition, the desired topic is registered.
* =================================================================
*/
void reconnectMQTT()
{
  while(!mqttClient.connected())
  {
    writeAdvanceDiag("Login to MQTT-Broker", true);
    if(mqttClient.connect(clientID.c_str()))
    {
      Serial.println("Connected to MQTT-Broker " + String(MQTT_BROKER));
    }
    else
    {
      writeAdvanceDiag("Failed with rc=" +String(mqttClient.state()), true);
      Serial.println("Next MQTT-Connect in 3 sec");
      delay(3000);
    }
  }
}

/*
* =================================================================
* Function:     writeAdvanceDiag   
* Returns:      void
* Description:  Writes advance msg to serial monitor, if
*               ADVANCEDIAG >= 1
* msg:          Message for the serial monitor
* newLine:      Message with linebreak (true)      
* =================================================================
*/
void writeAdvanceDiag(String msg, bool newLine)
{
  if(bool(ADVANCEDIAG)) //Check if advance diag is enabled
  {
    if(newLine)
      Serial.println(msg);
    else
      Serial.print(msg);
  }
}
