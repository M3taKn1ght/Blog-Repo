//-----------------------------------------------------
// Game "CYCLONE" for Arduino
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 20. Sep 2020
// Update:  25. Sep 2020
//-----------------------------------------------------
//Include libraries
#include <Adafruit_NeoPixel.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//Defines
#define NUMPIXELS   12  // Popular NeoPixel ring size or edit the number of LEDs
#define PIN         2   // Data-Pin to ring or strip
#define PINBTN      6   // Pin for Player-button
#define PINSCORERST 9   // Pin to reset score during first run

#define DISABLEWINDOW 3 //Rounds before the LED before and after target is not valid anymore

//Player-Dot speed defines
#define STARTINTERVAL 250 //"Normal" move
#define MAXINTERVAL 500   //Very slow move
#define MININTERVAL 50    //Very fast move

//Create objects
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD adress
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //Init NeoPixel object

bool bFirstRun, bSecureWindow;
int iState = 1;
int iTargetPos, iPlayerPos, iStoredHighscore, iRound, iScore, iInterval;  //Vars for the game
int iLastButtonPressed, iButtonState, iDebounceButton;  //Vars to debounce button
unsigned long iLastPlayerMove, ulLastDebounceTime;      //Timer to debouce button
unsigned long ulDebounceButton = 10;                    //Debounce-time

void setup() {
  Serial.begin(115200);
  Serial.println("Init serial communication: DONE");
  
  //Begin init for WS218B-ring or -strip
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixels to "off"
  pixels.setBrightness(20); // Set brightness to 20%
  pixels.show();   // Send the updated pixel colors to the hardware.
  Serial.println("Init WS218B-ring: DONE");
  
  //Begin init display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial.println("Init LCD display: DONE");
  
  randomSeed(analogRead(0));  // Make randome more randome
  Serial.println("Make randome more randome: DONE");

  //Read latest highscore from EEPROM
  iStoredHighscore = EEPROM.read(0);
  Serial.println("Last stored highscore: " + String(iStoredHighscore));

  //Init button with internal pullup-resistor
  pinMode(PINBTN,INPUT_PULLUP); //GameBTN
  pinMode(PINSCORERST,INPUT_PULLUP); //Reset-Pin for score
  
  //Init some basic-vars
  bFirstRun = true; //Enable firstrun
  iLastButtonPressed = digitalRead(PINBTN); //Init iLastButtonPressed
  iButtonState = digitalRead(PINBTN); //Init iButtonstate
}

void loop() {
  int iDebounceButton = DebounceButton(); //Check and debounce button

  if(!bFirstRun)
  {
    if(iState == 1) //Startscreen
    {
      bSecureWindow = true;
      iRound = 1;
      iScore = 0;
      iInterval = STARTINTERVAL;
      lcd.clear();
      lcd.home();
      lcd.print("Highscore: " + String(iStoredHighscore));
      lcd.setCursor(0,1);
      lcd.print("Press button ...");
      iState = 2;
    }
    if(iState == 2) //Get Button pressed
    {
      if(iDebounceButton == LOW)
      {
        if(iRound == 1) //Only show once during game
          Serial.println("-------- New game --------");
        lcd.clear();
        lcd.home();
        lcd.print("Release button");
        lcd.setCursor(0,1);
        lcd.print("to start");
        iState = 3; 
      }
    }
    if(iState == 3) //Init next round
    {
      if(iDebounceButton == HIGH)
      {
        lcd.clear();
        lcd.home();
        lcd.print("Round: " + String(iRound));
        Serial.println("Round: " + String(iRound));
        lcd.setCursor(0,1);
        lcd.print("Score: " + String(iScore));
        Serial.println("Score: " + String(iScore));
        iTargetPos = random(0,NUMPIXELS-1);
        Serial.println("New target pos: " + String(iTargetPos));
        iPlayerPos = random(0,NUMPIXELS-1);
        while(iTargetPos == iPlayerPos)
          iPlayerPos = random(0,NUMPIXELS-1);
        Serial.println("Player start pos: " + String(iPlayerPos));
        iState = 4;
      }
    }
    if(iState == 4) //Draw target and playes dot
    {
      DrawNextTarget(iTargetPos, bSecureWindow);  //Draw new target
      DrawPlayer(iPlayerPos);                     //Draw player dot
      iLastPlayerMove = millis();                 //Update timer for moving
      iState = 5;
    }
    if(iState == 5) //Wait pressing button and move player dot
    {
      if(iDebounceButton == LOW)
      {
        iState = 6;
      }
      else
      {
        unsigned long currentMillis = millis();
        if(currentMillis - iLastPlayerMove > iInterval)
        {
          iPlayerPos++;
          if(iPlayerPos >= NUMPIXELS)
            iPlayerPos = 0;
          DrawNextTarget(iTargetPos, bSecureWindow);
          DrawPlayer(iPlayerPos);
          iLastPlayerMove = currentMillis;
        }
      }
    }
    if(iState == 6) //Check if player win
    {
      if(CheckPlayerPos())  //Winner or loser?
      {
        iScore++; //Update score
        iRound++; //Update rounds
        iState = 2; //Go back to release button
        if(iRound > DISABLEWINDOW)  //Only target
        {
          bSecureWindow = false;
          iInterval = random(MININTERVAL,MAXINTERVAL);
        }
        else
          iInterval = random(STARTINTERVAL-50,MAXINTERVAL);
        Serial.println("New interval: " + String(iInterval));
      }
      else
        iState = 90;
    }
    if(iState == 90)  //Game ends
    {
      Serial.println("Game ends");
      lcd.clear();
      lcd.home();
      iDebounceButton = HIGH;
      iLastButtonPressed = HIGH;
      iButtonState = HIGH;
      if(iScore > iStoredHighscore) //New highscore?
      {
        lcd.print("New highscore ");
        lcd.setCursor(0,1);
        lcd.print("New score: " + String(iScore));
        Serial.println("New highscore is " + String(iScore));
        EEPROM.write(0,iScore); //Store new highscore to EEPROM
        iStoredHighscore = iScore;
      }
      else  //Loser
      {
        lcd.print("Game Over");
        lcd.setCursor(0,1);
        lcd.print("You lose");
        Serial.println("You lose!");
      }
      Serial.println("-------- End game --------");
      delay(2000);
      iState = 1;
    }
  }
  else
    InitFirstRun(); //Init Firstrun to check LCD and WS218B-ring
}

//Function to make first run
void InitFirstRun()
{
  if(digitalRead(PINSCORERST) == LOW) //Overwrite EEPROM with "0"
  {
    Serial.println("Reset highscore");
    for(int iCnt = 0; iCnt < EEPROM.length(); iCnt++)
      EEPROM.write(iCnt,0);
  }
  Serial.println("---- Start init ----");
  lcd.home();
  lcd.print("Game Cyclone");
  Serial.println("Game Cyclone");
  lcd.setCursor(0,1);
  lcd.print("(c)  M3taKn1ght");
  Serial.print("(c)  M3taKn1ght");
  delay(1000);
  lcd.clear();
  lcd.home();
  lcd.print("For az-Delivery");
  Serial.println("For az-Delivery");
  lcd.setCursor(0,1);
  lcd.print("Testing ring ...");
  Serial.println("Testing ring ...");
  delay(1000);
  pixels.clear();
  //Check every single LED
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(i,0,0);
    delay(50);
  }
  pixels.clear();
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(0,i,0);
    delay(50);
  }
  pixels.clear();
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(0,0,i);
    delay(50);
  }
  pixels.clear();
  pixels.show();
  Serial.println("----  End init  ----");
  bFirstRun = false;
  Serial.println("bFirstRun: " + String(bFirstRun));
  Serial.println("Activate state for game");
}

//Simple function to check LED-Ring one by one
void InitRingTest(int iRed, int iGreen, int iBlue)
{
  Serial.println("R: " + String(iRed) + " G: " + String(iGreen) + " B: " + String(iBlue));
  for(int iPixel = 0; iPixel < NUMPIXELS; iPixel++)
  {
    pixels.setPixelColor(iPixel, pixels.Color(iRed, iGreen, iBlue));
    pixels.show();
    delay(50);
  }
}

//Function to draw target an secure area for player
void DrawNextTarget(int iPos, bool bArea)
{
  pixels.clear();
  pixels.setPixelColor(iPos, pixels.Color(0, 255, 0));
  if(bArea)
  {
    if(iPos - 1 < 0)
      pixels.setPixelColor(NUMPIXELS - 1, pixels.Color(255, 136, 0));
    else
      pixels.setPixelColor(iPos - 1, pixels.Color(255, 136, 0));

    if(iPos + 1 >= NUMPIXELS)
      pixels.setPixelColor(0, pixels.Color(255, 136, 0));
    else
      pixels.setPixelColor(iPos + 1, pixels.Color(255, 136, 0));
  }
}

//Function to draw players LED
void DrawPlayer(int iPos)
{
  if(iPos == iTargetPos)  //target and player-dot is equal 
    pixels.setPixelColor(iPos, pixels.Color(0, 0, 255));  //Dot color will blue
  else
    pixels.setPixelColor(iPos, pixels.Color(255, 0, 0));  //Otherwise red
  pixels.show();
}

//Function to check after pressing button, if user hit the target
bool CheckPlayerPos()
{
  if(iTargetPos == iPlayerPos)  //Player hit target?
    return true;
  else
  {
    if(bSecureWindow) //LED before and after target active?
    {
      int iBeforeTarget = iTargetPos - 1;
      int iAfterTarget = iTargetPos + 1;
      if(iBeforeTarget < 0)
        iBeforeTarget = NUMPIXELS - 1;
      if(iAfterTarget >= NUMPIXELS)
        iAfterTarget = 0;
      if(iBeforeTarget == iPlayerPos || iAfterTarget == iPlayerPos)
        return true; 
      else
        return false;
    }
    else
      return false;
  }
}

//Function to debounce button
int DebounceButton()
{
  int iCurrentButtonState = digitalRead(PINBTN);
  if(iCurrentButtonState != iLastButtonPressed)
    ulLastDebounceTime = millis();

  if((millis() - ulLastDebounceTime) > ulDebounceButton)
  {
    if(iCurrentButtonState != iButtonState)
      iButtonState = iCurrentButtonState;
  }
  iLastButtonPressed = iCurrentButtonState;
  return iButtonState;
}
