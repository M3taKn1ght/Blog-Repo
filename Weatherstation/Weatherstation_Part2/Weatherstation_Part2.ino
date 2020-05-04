// Weatherstation Part 2 -- 2020 Joern Weise GPL 3.0 --

#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ArduinoJson.h>  
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

//Variables for display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

//Login data for WiFi
const char *ssid     = "WIFI_SSID";   //TODO
const char *password = "WIFI-PASS";   //TODO

//Needed variables for openweathermap.org
const String apiKey = "ADD-YOUR_API";         //TODO
const String location = "ADD-YOUR-LOCATION";  //TODO like "Wiesbaden,de"
const char *clientAdress = "api.openweathermap.org";
int strMinTemp, strMaxTemp, strCurTemp, strFeelTemp, strSunrise, strSunset;
DynamicJsonDocument jsonDoc(2000);

//Variables to get and set time
int utcOffsetInSeconds = 7200;
String daysOfTheWeek[7] = {"So","Mo", "Di", "Mi", "Do", "Fr", "Sa"};
time_t tmLastFullMoon;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

bool firstrun = true;
bool bUpdateDisplay = true;

//Setup to init the NodeMCU
void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Address 0x3C for OLED-Display
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  tmLastFullMoon = tmConvert(YEAR,MONTH,DAY,HOUR,MINUTE,SECOND); //TODO https://www.mondverlauf.de
  Serial.println("");
  Serial.println("Last moon: " + String(tmLastFullMoon));
  Serial.println("Connected to Wifi with IP: " + WiFi.localIP().toString());
  timeClient.begin();
}

void loop() {

  WeatherUpdate();  //Method to update weather
  TimeUpdate();     //Method to update time
  DisplayUpdate();  //Method to update Display
  if(firstrun)
    firstrun = false;
}

//Method to update time and overwrite 
void TimeUpdate()
{
  static bool bUpdateDone;
  if((firstrun || (minute() % 2) == 0) && !bUpdateDone)
  {
    bool bUpdate = timeClient.update();
    setTime(timeClient.getEpochTime());
    Serial.print("Update time: ");
    if(bUpdate){
      Serial.println("Success");
    }
    else{
     Serial.println("Failed");
    }
    bUpdateDone = true;
  }
  if((minute() % 2) != 0)
    bUpdateDone = false;
}

//Method to update weather forecast
void WeatherUpdate()
{
  static bool bUpdateDone;
  if((firstrun || (minute() % 5) == 0) && !bUpdateDone)
  {
    jsonDoc.clear();  //Normally not needed, but sometimes new data will not stored
    String strRequestData = RequestWeather(); //Get JSON as RAW string
    Serial.println("Received data: " + strRequestData);
    //Only do an update, if we got valid data
    if(strRequestData != "")  //Only do an update, if we got valid data
    {
      DeserializationError error = deserializeJson(jsonDoc, strRequestData); //Deserialize string to AJSON-doc
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      //Save data to global var
      strMinTemp = RoundTemp(jsonDoc["main"]["temp_min"].as<double>());
      strMaxTemp = RoundTemp(jsonDoc["main"]["temp_max"].as<double>());
      strCurTemp = RoundTemp(jsonDoc["main"]["temp"].as<double>());
      strFeelTemp = RoundTemp(jsonDoc["main"]["feels_like"].as<double>());
      strSunrise  = jsonDoc["sys"]["sunrise"].as<int>();
      strSunset   = jsonDoc["sys"]["sunset"].as<int>();
      static long timeZone = jsonDoc["timezone"];  //Get latest timezone
      //Print to Serial Monitor
      Serial.println("Min Temp: " + strMinTemp);
      Serial.println("Max Temp: " + strMaxTemp);
      Serial.println("Cur Temp: " + strCurTemp);
      Serial.println("Feel Temp: " + strFeelTemp);
      //Check if timezone changed (sommer- / wintertime
      if(timeZone != utcOffsetInSeconds)
      {
        utcOffsetInSeconds = timeZone;
        timeClient.setTimeOffset(utcOffsetInSeconds);
      }
    }
    bUpdateDone = true;
    bUpdateDisplay = true;
    }
  if((minute() % 5) != 0)
   bUpdateDone = false;
}

//Method for the API-Request to openweathermap.org
String RequestWeather()
{
  WiFiClientSecure client;
  if(!client.connect(clientAdress,443)){  //Changed to https -> Port 443
    Serial.println("Failed to connect");
    return "";
  }
  /*
   * path as followed:
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

int RoundTemp(double dTemp)
{
  return int(dTemp + 0.5);
}

//Method to update display content
void DisplayUpdate()
{
  static int iLastMinute;
  if(iLastMinute != minute() || firstrun || bUpdateDisplay){
    display.clearDisplay();
    display.setTextSize(1);               // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setCursor(0,0);               // Start at top-left corner
    display.println(String(daysOfTheWeek[weekday()-1]) + " " + GetDigits(day()) +
                    String(".") + GetDigits(month()) + String(".") + year() + 
                    "  " + GetDigits(hour()) + String(":") + GetDigits(minute()));
    display.println(String("Minimum  ") + strMinTemp + String(" C"));
    display.println(String("Maximum  ") + strMaxTemp + String(" C"));
    display.println(String("Aktuell   ") + strCurTemp + String(" C"));  //"Aktuell" german for current
    display.println(String("Gefuehlt  ") + strFeelTemp + String(" C")); //"Gefuehlt" german for feels like
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    display.write(int16_t(30));
    display.println(" Sonne  " + GetTimeAsString(strSunrise, utcOffsetInSeconds));
    display.write(int16_t(31));
    display.println(" Sonne  " + GetTimeAsString(strSunset, utcOffsetInSeconds));
    display.println(GetMoonPhase());
    display.display();
    //Print all in serial monitor
    Serial.println("---------------------");
    Serial.println("EpochTime: " + String(timeClient.getEpochTime()));
    Serial.println(String(daysOfTheWeek[weekday()-1]) + String(". ") +
                  GetDigits(day()) + String(".") + GetDigits(month()) + 
                  String(".") + year());
    Serial.println(GetDigits(hour()) + String(":") + GetDigits(minute()));
    Serial.println(String("Min: ") + strMinTemp);
    Serial.println(String("Max: ") + strMaxTemp);
    Serial.println(String("Current: ") + strCurTemp);
    Serial.println(String("Feels like: ") + strFeelTemp);
    Serial.println(String("Sunrise: ") + GetTimeAsString(strSunrise, utcOffsetInSeconds));
    Serial.println(String("Sunset:  ") + GetTimeAsString(strSunset, utcOffsetInSeconds));
    Serial.println(String("Moonphase: ") + GetMoonPhase());
    iLastMinute = minute();
    bUpdateDisplay = false;
  }
}

//Method to write given integer to String
//If the value is less than 10, a "0" is placed in front
String GetDigits(int iValue)
{
  String rValue = "";
  if(iValue < 10)
    rValue += "0";
  rValue += iValue;
  return rValue;
}

time_t tmConvert(int iYear, byte byMonth, byte byDay, byte byHour, byte byMinute, byte bySecond)
{
  tmElements_t tmSet;
  tmSet.Year = iYear - 1970;
  tmSet.Month = byMonth;
  tmSet.Day = byDay;
  tmSet.Hour = byHour;
  tmSet.Minute = byMinute;
  tmSet.Second = bySecond;
  return makeTime(tmSet);
}

String GetTimeAsString(int iValue,  int iOffset)
{
  if(iValue > 0){
    iValue += iOffset;
    return GetDigits(hour(iValue))+":"+GetDigits(minute(iValue));
  }
  return "";
}

double MoonUpdate()
{
    double dDaySinceLastFullmoon = (now() / 86400 - tmLastFullMoon / 86400) / 29.53;
    int iToHundret = (dDaySinceLastFullmoon - int(dDaySinceLastFullmoon)) * 100;
    return (double) iToHundret / 100; //Needed to get only 2 decimal places
}

String GetMoonPhase()
{
    static double dMoonphase = MoonUpdate();     //Method to update moonphase
    Serial.println("Calculated moon result: " +String(dMoonphase,3));
    if(dMoonphase == 0.0) 
      dMoonphase = 1.00;
      
    if(dMoonphase < double(0.25))
    {
      return "Abnehmende Mond";
    }
    else if (dMoonphase == 0.25)
    {
      return "Abnehmender Halbmond";
    }
    else if(0.25 < dMoonphase && dMoonphase < 0.50)
    {
      return "Abnehmende Sichel";
    }
    else if(dMoonphase == 0.50)
    {
      return "Neumond";
    }
    else if(0.50 < dMoonphase && dMoonphase < 0.75)
    {
      return "Zunehmende Sichel";
    }
    else if(dMoonphase == 0.75)
    {
      return "Zunehmender Halbmond";
    }
    else if(0.75 < dMoonphase && dMoonphase < 1.00)
    {
      return "Zunehmender Mond";
    }
    else //Moonphase == 1
    {
      return "Vollmond";
    }
}
