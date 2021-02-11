//-----------------------------------------------------
// Example 3 NodeMCU with Poti transfer to mqtt-broker
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Oct 2020
// Update:  25. Oct 2020
//-----------------------------------------------------
#include <ESP8266WiFi.h>  //Lib for Wifi
#include <PubSubClient.h> //Lib for MQTT Pub and Sub

#ifndef STASSID
#define STASSID ""       //Enter Wfi-Name
#define STAPSK  ""  //Enter Wifi-Passkey
#endif
#define ADVANCEDIAG 1
#define MSG_BUFFER_SIZE  (50)
#define UPDATETIME 200
#define MINVALUECHANGE 1

const char* MQTT_BROKER = "raspberrypi";  //Name of the mqtt broker
String clientID = "NodeMCU_1";  //Clientname for MQTT-Broker
const char* PubTopicPoti = "/Client/ESP32/Poti/Value";   //Topic where to publish
const int iAnalogPin = A0;  //Set analog pin
int iSensorValue = 0;
int iLastValue = 0;

//Create objects for mqtt
WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned int iLastTime = 0;
char msg[MSG_BUFFER_SIZE];
void setup()
{
  Serial.begin(115200); //Start Serial monitor baudrate 115200
  delay(50);
  writeAdvanceDiag("SerialMonitor enabled", true);
  setupWifi();
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER, 1883);
  writeAdvanceDiag("Set Callback-function", true);
  writeAdvanceDiag("Finish setup()-Function", true);
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

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
    reconnectMQTT();

  mqttClient.loop();

  if (millis() - iLastTime > UPDATETIME)
  {
    iSensorValue = analogRead(iAnalogPin);  //Read analog value
    if (iSensorValue != iLastValue)
    {
      if (abs(iSensorValue - iLastValue) > MINVALUECHANGE) // Check if change is hihg enough
      {
        Serial.println("Sensorvalue: " + String(iSensorValue));
        snprintf(msg, MSG_BUFFER_SIZE, "%d", iSensorValue); //Convert message to char
        mqttClient.publish(PubTopicPoti, msg); //Send to broker
        iLastValue = iSensorValue;
      }
    }
    iLastTime = millis();
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
