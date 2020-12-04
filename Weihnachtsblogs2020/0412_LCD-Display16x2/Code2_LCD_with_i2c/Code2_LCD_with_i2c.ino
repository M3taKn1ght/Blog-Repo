//-----------------------------------------------------
// LiquidCrystal I2C connection
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 24. Nov 2020
// Update:  24. Nov 2020
//-----------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

  
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();      // initialize the lcd 
  lcd.backlight(); // backlight on
  
  lcd.home(); // set cursor to 0,0

  //Write some text
  lcd.print(" LCD-Display  ");
  lcd.setCursor(0, 1);
  lcd.print(" with i2c ");
  
}
//-----------------------------------------------------
// LiquidCrystal I2C connection
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 24. Nov 2020
// Update:  24. Nov 2020
//-----------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

  
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();      // initialize the lcd 
  lcd.backlight(); // backlight on
  
  lcd.home(); // set cursor to 0,0

  //Write some text
  lcd.print(" LCD-Display  ");
  lcd.setCursor(0, 1);
  lcd.print(" with i2c ");
  
}

void loop()
{

}

void loop()
{

}
