//-----------------------------------------------------
// Analog clock for Az-Touch Mod 2.4"-Display
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 19. Jul 2021
// Update:  21. Jul 2021
//-----------------------------------------------------

/* --- Some color definitions, feel free to change --- */
#define FACE            TFT_ORANGE
#define NUMERIC_POINT   TFT_BLACK
#define BACKGROUND      TFT_WHITE
#define HOURCOLOR       TFT_RED
#define MINUTECOLOR     TFT_BLACK

/* --- Needed libs for this project --- */
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ArduinoJson.h> 
#include <WiFiClientSecure.h>
#include "bitmap.h"   //This is a file in the same dir from this program(!!!)

TFT_eSPI tft = TFT_eSPI();

const char* ssid       = "HERE YOUR WIFI-SSID";       //CHANGE SSID from network
const char* password   = "HERE YOUR WIFI-PASS";  //CHANGE Password from network

/*--- Needed variables for openweathermap.org ---*/
const String apiKey = "HERE API-KEY FROM openweathermap.org"; 	//CHANGE API-Key from openweathermap.org
const String location = "HERE LOCATION FOR openweathermap.org"; //CHANGE Location for openweathermap.org e.g. Wiesbaden,de
const char *clientAdress = "api.openweathermap.org";

DynamicJsonDocument jsonDoc(20000); //Var to parse JSON-Data

/*--- Define NTP Client to get time ---*/
const char* ntpServer = "europe.pool.ntp.org";  //Use Europe-NTP-Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, 0);
String daysOfTheWeek[7] = {"Sa", "So", "Mo", "Di", "Mi", "Do", "Fr"}; //Will be used on SerialMonitor
unsigned long utcOffsetInSeconds = 7200;


/*--- Some baisc variables for the code ---*/
int pPrevmin, pPrevhour, pPrevSecond;
String strSunset, strSunrise;
bool bFirstrun, bUpdateSunSetRise;

/*--- Basicparams for clock ---*/
#define Xo 110   // center point in X
#define Yo 120   // center point in Y
#define RADIUS 80  // radius of the clock face

/*
* =================================================================
* Function:     setup   
* Returns:      void
* Description:  Setup display and sensors
* =================================================================
*/
void setup()
{
  bFirstrun = true;
  bUpdateSunSetRise = false;
  bool bTempReturn;
  bool bFail = false;
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  Serial.begin(115200);
  Serial.println("Analog clock for Az-Touch by Joern Weise");
  Serial.println("For Az-Touch Mod 2.4 and 2.8 -Display");
  randomSeed(analogRead(34));

  /*--- Init TFT and write some text ---*/
  tft.init();
  tft.setRotation(3);
  tft.fillScreen((BACKGROUND));
  tft.setCursor(15, 20, 2);
  tft.setTextColor(TFT_RED, TFT_WHITE);
  tft.setTextSize(2);
  tft.println("Analog clock Az-Touch");
  tft.setCursor(40, 60, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.print("Display : ");
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
  tft.println("DONE");

  
  /*--- Init WiFi ---*/
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor(40, 100, 2);
  tft.print("WiFi : ");
  WiFi.begin(ssid, password);
  static unsigned long ulSeconds = second();
  while ( WiFi.status() != WL_CONNECTED)
  {
    delay ( 500 );
    Serial.print ( "." );
    if(second() - ulSeconds >= 30)
      break;
  }
  
  if(WiFi.status() == WL_CONNECTED)
  {
    tft.setTextColor(TFT_GREEN, TFT_WHITE);
    tft.println("DONE");
  }
  else
  {
    bFail = true;
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.println("FAIL");
  }
      
  /*--- Update needed time and data ---*/
  timeClient.begin();
  if(!bFail)
  {
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setCursor(40, 140, 2);
    tft.print("Weatherdata : ");
    bTempReturn = UpdateOpenWeatherMapData();
    bFail = bFail || !bTempReturn;
    if(bTempReturn)
    {
      tft.setTextColor(TFT_GREEN, TFT_WHITE);
      tft.println("DONE");
    }
    else
    {
      tft.setTextColor(TFT_RED, TFT_WHITE);
      tft.println("FAIL");
    }
  }
  if(!bFail)
  {
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setCursor(40, 180, 2);
    tft.print("NTP update : ");
    bTempReturn = UpdateNTPTime();
    bFail = bFail || !bTempReturn;
    if(bTempReturn)
    {
      tft.setTextColor(TFT_GREEN, TFT_WHITE);
      tft.println("DONE");
    }
    else
    {
      tft.setTextColor(TFT_RED, TFT_WHITE);
      tft.println("FAIL");
    }
  }
  ulSeconds = second();
  while(second() - ulSeconds < 5 && !bFail){}
  tft.fillScreen((BACKGROUND));
  /*--- Draw clock and bitmap ---*/
  drawFace();
  tft.drawBitmap(230,20,sunrise,64,64,TFT_BLACK);
  tft.drawBitmap(230,120,sunset,64,64,TFT_BLACK);
  
}

/*
* =================================================================
* Function:     loop   
* Returns:      void
* Description:  Main loop to let program work
* =================================================================
*/
void loop()
{
  DrawTime();
  UpdateOpenWeatherMapData();
  UpdateNTPTime();
  UpdateSunriseSunset();

  //Serial output
  if(pPrevSecond != second() || bFirstrun)
  {
    Serial.print(daysOfTheWeek[weekday()] + "., ");
    Serial.print(GetDigits(hour()) );
    Serial.print(":");
    Serial.print(GetDigits(minute()) );
    Serial.print(":");
    Serial.println(GetDigits(second()) );
    pPrevSecond = second();
  }

  if(bFirstrun)
    bFirstrun = false;
}

/*
* =================================================================
* Function:     UpdateNTPTime   
* Returns:      bool
* Description:  Sync ESP-Time with NTP
* =================================================================
*/
bool UpdateNTPTime()
{
  static bool bUpdateDone;
  if((bFirstrun || (minute() % 15) == 0) && !bUpdateDone)
  {
    bool bUpdate = timeClient.update();
    Serial.print("Update time: ");
    if(bUpdate)
    {
      unsigned long ulNTPTime = timeClient.getEpochTime();
      timeClient.setTimeOffset(utcOffsetInSeconds);
      setTime(timeClient.getEpochTime());
      Serial.println("SUCCESS");
      bUpdateDone = true;
    }
    else
    {
      Serial.println("FAILED");
      bUpdateDone = false;
      return false;
    }
  }
  if((minute() % 2) != 0)
    bUpdateDone = false;
  return true;
}

/*
* =================================================================
* Function:     UpdateOpenWeatherMapData   
* Returns:      bool
* Description:  Update OpenWeathermap data
* =================================================================
*/
bool UpdateOpenWeatherMapData()
{
  static bool bUpdateDone;
  if((bFirstrun || (minute() % 30) == 0) && !bUpdateDone)
  {
    jsonDoc.clear();  //Normally not needed, but sometimes new data will not stored
    String strRequestData = RequestData(); //Get JSON as RAW string
    Serial.println("Received data: " + strRequestData);
    //Only do an update, if we got valid data
    if(strRequestData != "")  //Only do an update, if we got valid data
    {
      DeserializationError error = deserializeJson(jsonDoc, strRequestData); //Deserialize string to AJSON-doc
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
      }
      //Save data to global var
      static unsigned long timeZone = jsonDoc["timezone"];  //Get latest timezone
      //Print to Serial Monitor
      time_t ttSunrise, ttSunset;
      ttSunrise  = jsonDoc["sys"]["sunrise"].as<time_t>();  //Convert seconds to time_t
      ttSunset   = jsonDoc["sys"]["sunset"].as<time_t>();   //Convert seconds to time_t
      Serial.println("SunRise: " + String(hour(ttSunrise+utcOffsetInSeconds)) + ":" + String(minute(ttSunrise+utcOffsetInSeconds)) );
      Serial.println("SunSet: " + String(hour(ttSunset+utcOffsetInSeconds)) + ":" + String(minute(ttSunset+utcOffsetInSeconds)) );
      //Check if sunset or sunrise has changed
      if(strSunset != String(GetDigits(hour(ttSunset+utcOffsetInSeconds))) + ":" + String(GetDigits(minute(ttSunset+utcOffsetInSeconds))) )
      {
        strSunset = String(GetDigits(hour(ttSunset+utcOffsetInSeconds))) + ":" + String(GetDigits(minute(ttSunset+utcOffsetInSeconds)));
        bUpdateSunSetRise = true;
      }
      if(strSunrise != String(GetDigits(hour(ttSunrise+utcOffsetInSeconds))) + ":" + String(GetDigits(minute(ttSunrise+utcOffsetInSeconds))) )
      {
        strSunrise = String(GetDigits(hour(ttSunrise+utcOffsetInSeconds))) + ":" + String(GetDigits(minute(ttSunrise+utcOffsetInSeconds)));
        bUpdateSunSetRise = true;
      }
      /*--- Update timezone ---*/
      if(timeZone != utcOffsetInSeconds)
      {
        utcOffsetInSeconds = timeZone;
        timeClient.setTimeOffset(utcOffsetInSeconds);
      }
      bUpdateDone = true;
    }
    else
      return false;
    }
  if((minute() % 2) != 0)
   bUpdateDone = false;
 return true;
}

/*
* =================================================================
* Function:     RequestData   
* Returns:      String
* Description:  Request to openWeathermap.org to get latest data
* =================================================================
*/
String RequestData()
{
  WiFiClientSecure client;
  if(!client.connect(clientAdress,443)){
    Serial.println("Failed to connect");
    return "";
  }
  /*
   * path as followed, see documentation:
   * /data/2.5/weather? <- static url-path
   * q="location"       <- given location to get weatherforecast
   * &lang=de           <- german description for weather
   * &units=metric      <- metric value in Celcius and hPa
   * appid="apiKey"     <- API-Key from user-account
   */
  String path = "/data/2.5/weather?q=" + location + "&lang=de&units=metric&appid=" + apiKey;

  //Send request to openweathermap.org
  client.print(
    "GET " + path + " HTTP/1.1\r\n" + 
    "Host: " + clientAdress + "\r\n" + 
    "Connection: close\r\n" + 
    "Pragma: no-cache\r\n" + 
    "Cache-Control: no-cache\r\n" + 
    "User-Agent: ESP32\r\n" + 
    "Accept: text/html,application/json\r\n\r\n");

  //Wait for the answer, max 2 sec.
  uint64_t startMillis = millis();
  while (client.available() == 0) {
    if (millis() - startMillis > 2000) {
      Serial.println("Client timeout");
      client.stop();
      return "";
    }
  }

  //If there is an answer, parse answer from openweathermap.org
  String resHeader = "", resBody = "";
  bool receivingHeader = true;
  while(client.available()) {
    String line = client.readStringUntil('\r');
    if (line.length() == 1 && resBody.length() == 0) {
      receivingHeader = false;
      continue;
    }
    if (receivingHeader) {
      resHeader += line;
    }
    else {
      resBody += line;
    }
  }
  
  client.stop(); //Need to stop, otherwise NodeMCU will crash after a while
  return resBody;
}

/*
* =================================================================
* Function:     UpdateSunriseSunset   
* Returns:      String
* Description:  Update displayed text for sunset and sunrise
* =================================================================
*/
void UpdateSunriseSunset()
{
  if(bUpdateSunSetRise)
  {
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.fillRect(230,75,64,30,TFT_WHITE);
    tft.setCursor(230,75);
    tft.print(strSunrise);
    tft.fillRect(230,175,64,30,TFT_WHITE);
    tft.setCursor(230,175);
    tft.print(strSunset);
    bUpdateSunSetRise = false;
  }
}

/*
* =================================================================
* Function:     GetDigits   
* Returns:      String
* Description:  If the value is less than 10, a "0" is placed in front
* =================================================================
*/
String GetDigits(int iValue)
{
  String rValue = "";
  if(iValue < 10)
    rValue += "0";
  rValue += iValue;
  return rValue;
}

/*
* =================================================================
* Function:     drawFace   
* Returns:      String
* Description:  Draw clock face
* =================================================================
*/
void drawFace()
{
  int i = 0, angle = 0;
  float x, y;

  // Draw outer frame
  tft.drawCircle(Xo, Yo, RADIUS + 21, FACE);
  tft.drawCircle(Xo, Yo, RADIUS + 20, FACE);

  // Draw inner frame
  tft.drawCircle(Xo, Yo, RADIUS + 12, FACE);
  tft.drawCircle(Xo, Yo, RADIUS + 11, FACE);
  tft.drawCircle(Xo, Yo, RADIUS + 10, FACE);

  //Draw Numeric point

  for (i = 0; i <= 12; i++)
  {
    x = Xo + round(RADIUS * cos(angle * M_PI / 180) );
    y = Yo + round(RADIUS * sin(angle * M_PI / 180) );
    tft.drawCircle(x, y, 2, NUMERIC_POINT);
    angle += 30;
  }

  for (i = 0; i < 360; i += 6)
    tft.drawPixel(Xo + round(RADIUS * cos(i * M_PI / 180)), Yo + round(RADIUS * sin(i * M_PI / 180)), NUMERIC_POINT);
}

/*
* =================================================================
* Function:     DrawMinuteHand   
* Returns:      void
* Description:  Draw minute hand on clock face
* =================================================================
*/
void DrawMinuteHand(int m, int color)
{
  float angle=-90;
  float Xa, Ya;
  Xa=Xo+(RADIUS-20)*cos((angle+m*6)*M_PI/180);
  Ya=Yo+(RADIUS-20)*sin((angle+m*6)*M_PI/180);
  for(int iCnt = -1; iCnt <= 1; iCnt++)
    tft.drawLine(Xo, Yo+iCnt, Xa, Ya+iCnt, color);
}

/*
* =================================================================
* Function:     DrawHourHand   
* Returns:      void
* Description:  Draw hour hand on clock face
* =================================================================
*/
void DrawHourHand(int h, int m, int color)
{
  float angle=-90;
  float Xa, Ya;
  Xa=Xo+(RADIUS-(RADIUS/2))*cos((angle+h*30+(m*30/60))*M_PI/180);
  Ya=Yo+(RADIUS-(RADIUS/2))*sin((angle+h*30+(m*30/60))*M_PI/180);
  for(int iCnt = -1; iCnt <= 1; iCnt++)
   tft.drawLine(Xo, Yo+iCnt, Xa, Ya+iCnt, color);
}

/*
* =================================================================
* Function:     DrawCenterPoint   
* Returns:      void
* Description:  Draw centerpoint on clock face
* =================================================================
*/
void DrawCenterPoint()
{
  tft.fillCircle(Xo, Yo, 3, NUMERIC_POINT);
}

/*
* =================================================================
* Function:     DrawTime   
* Returns:      void
* Description:  Basefunction to draw hour and minute hands
* =================================================================
*/
void DrawTime()
{
  if(pPrevmin != minute() || pPrevhour != hour() || bFirstrun)
  {
    if(!bFirstrun)  //Only needed, cause otherwise esp crashs
    {
      DrawMinuteHand(pPrevmin, BACKGROUND);
      DrawHourHand(pPrevhour, pPrevmin, BACKGROUND);
    }
    DrawMinuteHand(minute(), MINUTECOLOR);
    DrawHourHand(hour(), minute(), HOURCOLOR);
    DrawCenterPoint();
    pPrevmin = minute();
    pPrevhour = hour();  
  }
}
