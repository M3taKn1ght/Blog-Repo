//-----------------------------------------------------
// Example 3 Arduino Uno Ehternetshiled to receive
// data from broker and show on LCD and LED 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 25. Oct 2020
// Update:  26. Oct 2020
//-----------------------------------------------------
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h> //Lib for MQTT Pub and Sub
#include <SPI.h>
#include <Ethernet.h>

#define ADVANCEDIAG 1

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const int iAnalogOut = 6; 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  //MAC-Adress for shield
IPAddress ip(192, 168, 178, 177);   //Static IP-Adress for arduino
IPAddress myDns(192, 168, 178, 1);  //IP-Adress router
char server[] = "www.google.com";   //Check if Arduino is online

String clientID = "Arduino_Uno";  //Clientname for MQTT-Broker
//Topics for subscribe
const char* MQTT_BROKER = "raspberrypi";  //Name of the mqtt broker
const char* SubTopicTempOne = "/Client/ESP32/TempOne";   //Topic first temp
const char* SubTopicTempTwo = "/Client/ESP32/TempTwo";   //Topic second temp
const char* SubTopicPresOne = "/Client/ESP32/PressOne";   //Topic first pressure
const char* SubTopicPresTwo = "/Client/ESP32/PressTwo";   //Topic second pressure
const char* SubTopicPotiMap = "/Client/ESP32/PotiMapValue";   //Topic mapped Poti

//Objects for ethernet-com
EthernetClient client;
PubSubClient mqttClient(client);

//Some vars for update
bool bUpdateDisplay = false;
bool bUpdatePWM = false;
int iLastTempOne,iLastTempTwo,iLastPressOne,iLastPressTwo,iLastPotiMap;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Arduino Uno - Monitor");
  pinMode(iAnalogOut, OUTPUT);
  Ethernet.init(10);  // Most Arduino shields use digital Pin 10
  lcd.init();       //Init LCD
  lcd.backlight();  //Backlight on
  lcd.clear();      //Clear old content
  bUpdateDisplay = true;
  Ethernet.begin(mac, ip);  //Init ethernet-shild
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  //Check if there is com to router
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  //Check if system is able to communicate 
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  //Init MQTT
  writeAdvanceDiag("Set MQTT-Server", true);
  mqttClient.setServer(MQTT_BROKER,1883);
  writeAdvanceDiag("Set Callback-function", true);
  mqttClient.setCallback(callback);
  writeAdvanceDiag("Finish setup()-Function", true);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!mqttClient.connected())
    reconnectMQTT();
  
  mqttClient.loop();

  if(bUpdateDisplay)
  {
    UpdateDisplay();
    bUpdateDisplay = false;
  }
    if(bUpdatePWM)
  {
    analogWrite(iAnalogOut, iLastPotiMap);  //Write new analog value to LED-Pin
    bUpdatePWM = false;
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
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicTempOne)+ "'", true);
      mqttClient.subscribe(SubTopicTempOne,1); //Subscibe topic "SubTopicTempOne"
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicTempTwo)+ "'", true);
      mqttClient.subscribe(SubTopicTempTwo,1); //Subscibe topic "SubTopicTempTwo"
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicPresOne)+ "'", true);
      mqttClient.subscribe(SubTopicPresOne,1); //Subscibe topic "SubTopicPresOne"
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicPresTwo)+ "'", true);
      mqttClient.subscribe(SubTopicPresTwo,1); //Subscibe topic "SubTopicPresTwo"
      writeAdvanceDiag("Subscribe topic '" + String(SubTopicPotiMap)+ "'", true);
      mqttClient.subscribe(SubTopicPotiMap,1); //Subscibe topic "SubTopicPotiMap"
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
  for (int i = 0; i < length; i++)
    stMessage += String((char)payload[i]);

  //Check if temp one has changed
  if(String(topic) == "/Client/ESP32/TempOne")
  {
    if(iLastTempOne != stMessage.toInt())
    {
      iLastTempOne = stMessage.toInt();
      bUpdateDisplay = true;
    }
  }
  //Check if temp two has changed
  if(String(topic) == "/Client/ESP32/TempTwo")
  {
    if(iLastTempTwo != stMessage.toInt())
    {
      iLastTempTwo = stMessage.toInt();
      bUpdateDisplay = true;
    }
  }
  //Check if pressure one has changed
  if(String(topic) == "/Client/ESP32/PressOne")
  {
    if(iLastPressOne != stMessage.toInt())
    {
      iLastPressOne = stMessage.toInt();
      bUpdateDisplay = true;
    }
  }
  //Check is pressure two has changed
  if(String(topic) == "/Client/ESP32/PressTwo")
  {
    if(iLastPressTwo != stMessage.toInt())
    {
      iLastPressTwo = stMessage.toInt();
      bUpdateDisplay = true;
    }
  }
  //Check if mapped poti value has changed
  if(String(topic) == "/Client/ESP32/PotiMapValue")
  {
    if(iLastPotiMap != stMessage.toInt())
    {
      iLastPotiMap = stMessage.toInt();
      bUpdatePWM = true;
    }
  }
}

/*
* =================================================================
* Function:     UpdateDisplay   
* Returns:      void
* Description:  Display new values on I2C-Display
* =================================================================
*/
void UpdateDisplay()
{
  lcd.clear();
  lcd.home();
  lcd.print("Temp one[C]: ");
  lcd.print(iLastTempOne);
  lcd.setCursor(0,1);
  lcd.print("Temp two[C]: ");
  lcd.print(iLastTempTwo);
  lcd.setCursor(0,2);
  lcd.print("Press one[hPa]: ");
  lcd.print(iLastPressOne);
  lcd.setCursor(0,3);
  lcd.print("Press two[hPa]: ");
  lcd.print(iLastPressTwo);
}
