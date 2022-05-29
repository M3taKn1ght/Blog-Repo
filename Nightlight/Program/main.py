from neopixel import Neopixel
from time import sleep, ticks_ms
from machine import Pin,Timer

## Setup
NUM_LEDS = 32
LED_PIN = 19
BTN_PIN = 10
SENSOR_PIN = 14 #For later feature
pixels = Neopixel(NUM_LEDS, 0, LED_PIN, "GRB")
pixels.brightness(30)
btn = Pin(BTN_PIN, Pin.IN, Pin.PULL_DOWN)

## Constants
DEFTIMEBTN = 15*60*1000 #Define for lights on
                                                                       0 = 50 #Timer colorchange
TIMECOLOROFF = 0 #Timer
red = pixels.colorHSV(0, 255, 255)
green = pixels.colorHSV(21845, 255, 255)
blue = pixels.colorHSV(43691, 255, 255)

## Variables
lastValue = ticks_ms()
lastActiveSet = ticks_ms()
bActiveLight = False
bButtonPressed = False
btnLastState = False
btnControl = False
tmLastUpdate = 0
hue = 0

#Thats a simple function to debounce btn
def btn_pressed():
    global btnLastState, tmLastUpdate
    btnState = False
    if btnLastState != btn.value():
        tmLastUpdate = ticks_ms()
    if (ticks_ms() - tmLastUpdate) > 100:
        btnState = btn.value()
        
    btnLastState = btn.value()
    return btnState

## Loop
while True:
  bButtonPressed = btn_pressed() #Check button pressed or not
  #Function-loop if button pressed
  if bButtonPressed and not btnControl:
      if not bActiveLight:
         TIMECOLOROFF =  DEFTIMEBTN
         bActiveLight = True
         lastActiveSet = ticks_ms()
      else:
        TIMECOLOROFF =  0
        bActiveLight = False  
      btnControl = True
  elif not bButtonPressed and btnControl:
      btnControl = False
  #Function-loop to recolor LEDs    
  if (ticks_ms() - lastValue) > TIMECOLORCHANGE and bActiveLight: 
      hue += 50
      if(hue > 65535):
          hue = 0
      color = pixels.colorHSV(hue, 255, 255)
      pixels.fill(color)
      pixels.show()
      lastValue = ticks_ms()
  elif not bActiveLight:
      hue = 0
      color = pixels.colorHSV(hue, 0, 0)
      pixels.fill(color)
      pixels.show()
  #Timer to set lights off
  if (ticks_ms() - lastActiveSet) > TIMECOLOROFF:
    bActiveLight = False
