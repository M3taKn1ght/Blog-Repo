//-----------------------------------------------------
// Example 1 NodeMCU with MQTT
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Oct 2020
// Update:  25. Oct 2020
//-----------------------------------------------------

#include <ESP8266WiFi.h>  //Lib for Wifi
#include <PubSubClient.h> //Lib for MQTT Pub and Sub
//
#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

#define ADVANCEDIAG 1
const char* MQTT_BROKER = "raspberrypi";  //Name of the mqtt broker
const char* SUBTOPIC = "/#";
String clientID = "NodeMCU_1";  //Clientname for MQTT-Broker
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() 
{
  Serial.begin(115200); //Start Serial monitor baudrate 115200
  delay(50);
  writeAdvanceDiag("SerialMonitor enabled", true);
  setupWifi();
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER,1883);
  writeAdvanceDiag("Set Callback-function", true);
  mqttClient.setCallback(callback);
  writeAdvanceDiag("Finish setup()-Function", true);
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

void loop() {
  // put your main code here, to run repeatedly:
  if(!mqttClient.connected())
    reconnectMQTT();

  mqttClient.loop();
}

/*
* =================================================================
* Function:     callback   
* Returns:      void
* Description:  Will automatical called, if a subscribed topic
*               has a new message
* topic:        Returns the topic, from where a new msg comes from
* payload:      The message from the topic
* length:       Length of the msg, important to get conntent
* =================================================================
*/
void callback(char* topic, byte* payload, unsigned int length)
{
  String stMessage = "";
  writeAdvanceDiag("Message arrived from topic: " + String(topic), true);
  writeAdvanceDiag("Message length: " + String(length), true);
  for (int i = 0; i < length; i++)
    stMessage += String((char)payload[i]);
  writeAdvanceDiag("Message is: " + stMessage, true);
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
      Serial.println("Connected to MQTT-Broker " +String(MQTT_BROKER));
      writeAdvanceDiag("Subscribe topic '" + String(SUBTOPIC)+ "'", true);
      mqttClient.subscribe(SUBTOPIC,1); //Subscibe topic "SUBTOPIC"
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
