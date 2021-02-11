//-----------------------------------------------------
// Example 3 ESP-NodeMCU with two BME transfer to 
// mqtt-broker and mapping analog input
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Oct 2020
// Update:  25. Oct 2020
//-----------------------------------------------------
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h> //Lib for MQTT Pub and Sub

//Define WiFi-Settings
#ifndef STASSID
#define STASSID ""       //Enter Wfi-Name
#define STAPSK  ""  //Enter Passkey
#endif

#define ADVANCEDIAG 1

#define I2C_SDA1 21
#define I2C_SCL1 22
#define I2C_SDA2 17
#define I2C_SCL2 16
#define NEXTUPDATE 2000

//Objects for I2C and BME
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
Adafruit_BME280 bmeOne;
Adafruit_BME280 bmeTwo;
unsigned long lastTime = 0;

const char* MQTT_BROKER = "raspberrypi";  //Name of the mqtt broker
const char* PubTopicTempOne = "/Client/ESP32/TempOne";        //Topic first temp
const char* PubTopicTempTwo = "/Client/ESP32/TempTwo";        //Topic second temp
const char* PubTopicPresOne = "/Client/ESP32/PressOne";       //Topic first pressure
const char* PubTopicPresTwo = "/Client/ESP32/PressTwo";       //Topic second pressure
const char* PubTopicPotiMap = "/Client/ESP32/PotiMapValue";   //Topic second pressure
const char* SUBTOPIC = "/Client/ESP32/Poti/Value";            //Topic subscribe poti value
String clientID = "ESP-DevKit_1";  //Clientname for MQTT-Broker

int iLastTempOne,iLastTempTwo,iLastPressOne,iLastPressTwo;

//Create objects for mqtt
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("BME280 test");
  Serial.println("Init both I2C-Connections");
  I2Cone.begin(I2C_SDA1, I2C_SCL1, 400000);
  I2Ctwo.begin(I2C_SDA2, I2C_SCL2, 400000);
  Serial.println("Make first BME talking to us");
  bool bStatus;
  //Init first sensor
  bStatus = bmeOne.begin(0x76, &I2Cone);
  if (!bStatus)
  {
    Serial.println("Could not find a valid BME280 - 1 sensor, check wiring!");
    while (1);
  }
  else
    Serial.println("Valid BME280 - 1 sensor!");
  
  //Init second sensor
  bStatus = bmeTwo.begin(0x76, &I2Ctwo);
  if (!bStatus)
  {
    Serial.println("Could not find a valid BME280 - 2 sensor, check wiring!");
    while (1);
  }
  else
    Serial.println("Valid BME280 - 2 sensor!");
  writeAdvanceDiag("Init Wifi", true);
  setupWifi();
  writeAdvanceDiag("Init Wifi - DONE", true);
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER,1883);
  writeAdvanceDiag("Set Callback-function", true);
  mqttClient.setCallback(callback);
  writeAdvanceDiag("Finish setup()-Function", true);
}

void loop() {
  // put your main code here, to run repeatedly:
  int iTempOne,iTempTwo,iPressOne,iPressTwo;
  if(!mqttClient.connected())
    reconnectMQTT();
  
  mqttClient.loop();
  //Check after "NEXTUPDATE" if values has changed
  if(millis() - lastTime > NEXTUPDATE)
  {
    iTempOne = int(bmeOne.readTemperature()); //Get temp one
    iTempTwo = int(bmeTwo.readTemperature()); //Get temp two
    iPressOne = int(bmeOne.readPressure() / 100.0F);  //Get press one
    iPressTwo = int(bmeTwo.readPressure() / 100.0F);  //get press two
    if(iTempOne != iLastTempOne)  //Check temp one changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, "%1d",iTempOne);  //Convert message to char
      mqttClient.publish(PubTopicTempOne,msg,true);  //Send to broker
      writeAdvanceDiag("Send Temp one: " + String(iTempOne), true);
      iLastTempOne = iTempOne;
    }
    if(iTempTwo != iLastTempTwo)  //Check temp two changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, "%1d",iTempTwo);  //Convert message to char
      mqttClient.publish(PubTopicTempTwo,msg,true);  //Send to broker
      writeAdvanceDiag("Send Temp two: " + String(iTempTwo), true);
      iLastTempTwo = iTempTwo;
    }
    if(iPressOne != iLastPressOne)  //Check pressure one changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, "%1d",iPressOne);  //Convert message to char
      mqttClient.publish(PubTopicPresOne,msg,true);  //Send to broker
      writeAdvanceDiag("Send Press one: " + String(iPressOne), true);
      iLastPressOne = iPressOne;
    }
    if(iPressTwo!= iLastPressTwo) //Check pressure two changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, "%1d",iPressTwo);  //Convert message to char
      mqttClient.publish(PubTopicPresTwo,msg,true);  //Send to broker
      writeAdvanceDiag("Send Press two: " + String(iPressTwo), true);
      iLastPressTwo = iPressTwo;
    }
    lastTime = millis();
  }

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
  //Map value and send the mapped value to mqtt broker
  int iValue,iMapValue;
  iValue = stMessage.toInt();
  iMapValue = map(iValue,0,1024,0,255);
  snprintf(msg,MSG_BUFFER_SIZE, "%1d",iMapValue);  //Convert message to char
  writeAdvanceDiag("Send mapped PotiValue: " + String(iMapValue), true);
  mqttClient.publish(PubTopicPotiMap,msg,true);  //Send to broker
  
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
