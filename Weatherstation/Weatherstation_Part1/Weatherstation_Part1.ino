// Weatherstation Part 1 -- 2020 Joern Weise GPL 3.0 --

#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ArduinoJson.h>  
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

//Variables for display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Login data for WiFi
const char *ssid     = "WLAN-172162";
const char *password = "2022569145297772";

//Needed variables for openweathermap.org
const String apiKey = "7791c94b70921020c6944934bd1ee54e";
const String location = "Wiesbaden,de";
const char *clientAdress = "api.openweathermap.org";
String strMinTemp, strMaxTemp, strCurTemp, strFeelTemp;
DynamicJsonDocument jsonDoc(20000);

//Variables to get and set time
long utcOffsetInSeconds = 7200;
char daysOfTheWeek[7][4] = {"Sa", "So","Mo", "Di", "Mi", "Do", "Fr"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

bool firstrun = true;
bool bUpdateDisplay = true;

//Setup to init the NodeMCU
void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Address 0x3C for 128x32
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("");
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
  if((firstrun || (second() % 30) == 0) && !bUpdateDone)
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
  if((second() % 30) != 0)
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
  WiFiClient client;
  const int httpPort = 80;
  if(!client.connect(clientAdress,httpPort)){
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

String RoundTemp(double dTemp)
{
  return String(int(dTemp + 0.5));
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
    //Print first line on display
    display.println(String(daysOfTheWeek[weekday()]) + " " + getDigits(day()) + 
                    String(".") + getDigits(month()) + String(".") + year() + 
                    "  " + getDigits(hour()) + String(":") + getDigits(minute()));
    //Print second line on display
    display.println(String("Min ") + strMinTemp + String("  Max ") + strMaxTemp);
    //Print third line on display
    display.println(String("Current ") + strCurTemp + String(" C"));
    //Print last line on display
    display.println(String("Feels like ") + strFeelTemp + String(" C"));
    display.display();
    //Print all in serial monitor
    Serial.println("---------------------");
    Serial.println("EpochTime: " + String(timeClient.getEpochTime()));
    Serial.println(String(daysOfTheWeek[weekday()]) + String(". ") + 
                  getDigits(day()) + String(".") + getDigits(month()) + 
                  String(".") + year());
    Serial.println(getDigits(hour()) + String(":") + getDigits(minute()));
    Serial.println(String("Min ") + strMinTemp + String("  Max ") + strMaxTemp);
    Serial.println(String("Current ") + strCurTemp);
    Serial.println(String("Feels like ") + strFeelTemp);
    iLastMinute = minute();
    bUpdateDisplay = false;
  }
}

//Method to write given integer to String
//If the value is less than 10, a "0" is placed in front
String getDigits(int iValue)
{
  String rValue = "";
  if(iValue < 10)
    rValue += "0";
  rValue += iValue;
  return rValue;
}
