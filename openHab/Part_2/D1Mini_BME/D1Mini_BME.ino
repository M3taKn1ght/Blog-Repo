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
#include <PubSubClient.h> //Lib for MQTT Pub and Sub
#include <ESP8266WiFi.h>

//Define WiFi-Settings
#ifndef STASSID
#define STASSID "WLAN-172162"       //Enter Wfi-Name
#define STAPSK  "2022569145297772"  //Enter Passkey
#endif

#ifndef MQTTUSER
#define MQTTUSER "sensors"       //Enter Wfi-Name
#define MQTTPASS "s3ns0r"  //Enter Passkey
#endif


#define ADVANCEDIAG 1

#define I2C_SDA D2
#define I2C_SCL D1
#define NEXTUPDATE 2000
#define TEMPLIMIT 85.00

//Objects for I2C and BME
Adafruit_BME280 bmeOne;
unsigned long lastTime = 0;

const char* MQTT_BROKER = "192.168.178.107";  //Name of the mqtt broker
const char* PubTopicTempOne = "sensors/out/D1_Mini_Office/TempOne";        //Topic first temp
const char* PubTopicPresOne = "sensors/out/D1_Mini_Office/PressOne";       //Topic first pressure
const char* PubTopicHumOne =  "sensors/out/D1_Mini_Office/HumOne";         //Topic first humidity
String clientID = "D1_Mini_Office";  //Clientname for MQTT-Broker

float fLastTempOne,fLastPressOne,fLastHumOne;

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
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("Make first BME talking to us");
  bool bStatus;
  //Init first sensor
  bStatus = bmeOne.begin(0x76);
  if (!bStatus)
  {
    Serial.println("Could not find a valid BME280 - 1 sensor, check wiring!");
    while (1);
  }
  else
    Serial.println("Valid BME280 - 1 sensor!");
  

  writeAdvanceDiag("Init Wifi", true);
  setupWifi();
  writeAdvanceDiag("Init Wifi - DONE", true);
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER,1883);
  writeAdvanceDiag("Set Callback-function", true);
  //mqttClient.setCallback(callback);
  writeAdvanceDiag("Finish setup()-Function", true);
}

void loop() {
  // put your main code here, to run repeatedly:
  float fTempOne,fPressOne,fHumOne;
  if(!mqttClient.connected())
    reconnectMQTT();
  
  mqttClient.loop();
  //Check after "NEXTUPDATE" if values has changed
  if(millis() - lastTime > NEXTUPDATE)
  {
    fTempOne = bmeOne.readTemperature(); //Get temp one
    if(fTempOne > float(TEMPLIMIT))
    {
      return;
    }
    fPressOne = bmeOne.readPressure() / 100.0F;  //Get press one
    fHumOne = bmeOne.readHumidity();
    if(fTempOne != fLastTempOne && abs(fLastTempOne - fTempOne) > 0.1)  //Check temp one changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, String(fTempOne,1).c_str());  //Convert message to char
      mqttClient.publish(PubTopicTempOne,msg,true);  //Send to broker
      writeAdvanceDiag("Send Temp one: " + String(fTempOne,1), true);
      fLastTempOne = fTempOne;
    }
    if(fPressOne != fLastPressOne && abs(fLastPressOne - fPressOne) > 0.5)  //Check pressure one changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, String(fPressOne,2).c_str());  //Convert message to char
      mqttClient.publish(PubTopicPresOne,msg,true);  //Send to broker
      writeAdvanceDiag("Send Press one: " + String(fPressOne,2), true);
      fLastPressOne = fPressOne;
    }
    if(fHumOne != fLastHumOne && abs(fLastHumOne - fHumOne) > 0.5)  //Check pressure one changed and send
    {
      snprintf(msg,MSG_BUFFER_SIZE, String(fHumOne,2).c_str());  //Convert message to char
      mqttClient.publish(PubTopicHumOne,msg,true);  //Send to broker
      writeAdvanceDiag("Send Hum one: " + String(fHumOne,2), true);
      fLastHumOne = fHumOne;
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
/*
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
*/
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
    if(mqttClient.connect(clientID.c_str(),MQTTUSER,MQTTPASS))
    {
      Serial.println("Connected to MQTT-Broker " +String(MQTT_BROKER));
      //writeAdvanceDiag("Subscribe topic '" + String(SUBTOPIC)+ "'", true);
      //mqttClient.subscribe(SUBTOPIC,1); //Subscibe topic "SUBTOPIC"
    }
    else
    {
      writeAdvanceDiag("Failed with rc=" +String(mqttClient.state()), true);
      Serial.println("Next MQTT-Connect in 3 sec");
      delay(3000);
    }
  }
}
