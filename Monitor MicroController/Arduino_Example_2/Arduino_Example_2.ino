// Watchdog second example for Arduino 
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 11. Sep 2020
// Update:  11. Sep 2020
//-----------------------------------------------------

#include <avr/wdt.h>

void setup() {
  Serial.begin(9600);                       //Activate serial com
  pinMode(LED_BUILTIN, OUTPUT);             //Activate included LED
  digitalWrite(LED_BUILTIN, LOW);           //Turn the LED off
  Serial.println("Arduino started.....");   //First message for monitor
  wdt_enable(WDTO_1S);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
  Serial.println("Set LED to HIGH");  // Serial information
  delay(300);                        // wait for a second
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off by making the voltage LOW
  Serial.println("Set LED to LOW");   // Serial information
  delay(300);                        // wait for a second
  Serial.println("Reset watchdog");   // Serial information
  wdt_reset();                        // Reset watchdog
}
