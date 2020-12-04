//-----------------------------------------------------
// LiquidCrystal both communications
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 24. Nov 2020
// Update:  24. Nov 2020
//-----------------------------------------------------
#include <LiquidCrystal.h>              //include the LiquidCrystal library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

  
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  //define LCD pins (RS, E, D4, D5, D6, D7)
LiquidCrystal_I2C lcdI2C(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  lcd.begin(16, 2);  //initializes the LCD and specifies the dimensions
  lcd.home();
  lcd.print("Az-delivery");
  lcd.setCursor(0, 1);
  lcd.print("wishes you");

  lcdI2C.init();                      // initialize the lcd 
  lcdI2C.backlight();
  
  lcdI2C.home();
  
  lcdI2C.print("merry");
  lcdI2C.setCursor(0, 1);
  lcdI2C.print("christmas");
}

void loop() {


}
