//-----------------------------------------------------
// Controlling a DC-Motor with Nano V3.0
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 23. Feb 2021
// Update:  23. Feb 2021
//-----------------------------------------------------

//PWM and motor configuration
// Motor A
const int motor1Pin1 = 9;
const int motor1Pin2 = 8;
const int iPWMPin = 10;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; //Wait until serial is available
  }
  Serial.println("Motor control with NANO V3.0");
  Serial.println("(c) Joern Weise for AZ-Delivery");
  Serial.println("--------------------");
  Serial.println("Set outputs");
  
  //Set pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(iPWMPin, OUTPUT);

  Serial.println("Setup finished");
}

void loop() {
  Serial.println("----- Motor clockwise speedup-----");
  digitalWrite(motor1Pin1,HIGH); // A = HIGH and B = LOW means the motor will turn right
  digitalWrite(motor1Pin2,LOW);
  for (int i=0; i<256; i+=5)
  {
    Serial.println("Speed:" + String(i));
    analogWrite(iPWMPin, i);
    delay(100);
  }
  
  Serial.println("----- Motor clockwise break-----");
  for (int i=255; i>0; i-=5)
  {
    Serial.println("Speed:" + String(i));
    analogWrite(iPWMPin, i);
    delay(100);
  }

  Serial.println("----- Motor counterclockwise speedup-----");
  digitalWrite(motor1Pin1,LOW); // A = HIGH and B = LOW means the motor will turn right
  digitalWrite(motor1Pin2,HIGH);
  for (int i=0; i<256; i+=5)
  {
    Serial.println("Speed:" + String(i));
    analogWrite(iPWMPin, i);
    delay(100);
  }
  Serial.println("----- Motor counterclockwise break-----");
  for (int i=255; i>0; i-=5)
  {
    Serial.println("Speed:" + String(i));
    analogWrite(iPWMPin, i);
    delay(100);
  }
}
