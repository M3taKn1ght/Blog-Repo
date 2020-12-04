//-----------------------------------------------------
// LiquidCrystal parallel interface
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 24. Nov 2020
// Update:  24. Nov 2020
//-----------------------------------------------------
#include <LiquidCrystal.h>              //include the LiquidCrystal library

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  //define LCD pins (RS, E, D4, D5, D6, D7)

void setup() {
  lcd.begin(16, 2);  //initializes the LCD and specifies the dimensions
  lcd.home();
  lcd.print(" LCD-Display  ");
  lcd.setCursor(0, 1);
  lcd.print(" without i2c ");
}

void loop()
{

}
