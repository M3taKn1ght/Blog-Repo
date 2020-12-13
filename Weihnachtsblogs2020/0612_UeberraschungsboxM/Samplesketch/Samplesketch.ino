//-----------------------------------------------------
// Little sample for suprisebox M from Az-delivery
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 27. Nov 2020
// Update:  27. Nov 2020
//-----------------------------------------------------
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include <LiquidCrystal_I2C.h>

#define SENSOR_PIN             A0
#define REFERENCE_RESISTANCE   8000
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    20
#define B_VALUE                3950
#define INTERVAL               1000

Thermistor* thermistor;
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const int PinDigitalTouch = 7;
const int PinLED = 13;
unsigned long iLastUpdate;
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  pinMode(PinDigitalTouch, INPUT);
  pinMode(PinLED, OUTPUT);
  thermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );
  lcd.init();      // initialize the lcd 
  lcd.backlight(); // backlight on
  
  lcd.home(); // set cursor to 0,0

  //Write some text
  lcd.print("Suprisebox");
  lcd.setCursor(0, 1);
  lcd.print("az-delivery.de");
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(PinDigitalTouch) == HIGH)
    digitalWrite(PinLED,HIGH);
  else
   digitalWrite(PinLED,LOW);
   
  int celsius = int(thermistor->readCelsius());
  int kelvin = int(thermistor->readKelvin());
  
  if(millis() - iLastUpdate > INTERVAL)
  {
    lcd.clear();
    lcd.home();
    lcd.print("Grad: " + String(celsius));
    lcd.setCursor(0,1);
    lcd.print("Kelvin: " + String(kelvin));
    Serial.print(String(celsius) + " C, ");
    Serial.println(String(kelvin) + " K, ");
    iLastUpdate = millis();
  }
}
