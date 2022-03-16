'''
// Read BME280 and
// show on i2c-OLED
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 23. Oct 2021
// Update:  24. Oct 2021
'''

 #First import needed libs
from ssd1306 import SSD1306_I2C #Import from Lib the needed subpart
from bme280 import BME280 #Import BME280-lib
from machine import Pin, I2C
import machine  #Important to get Pins and config them
import utime #For a little timer

#Definitions for internal temp-sensor
sensor = machine.ADC(4) # Create object sensor and init as pin ADC(4)
conversion_factor = 3.3 / 65535 # 3.3V are 16bit (65535)

#Definitons for i2c-Com
sda=Pin(0)
scl=Pin(1)
i2c=I2C(0, sda=sda, scl=scl, freq=400000)

#Write in cmd found addresses
i2cScan = i2c.scan()
counter = 0
for i in i2cScan:
    print('I2C Address ' + str(counter) + '      : '+hex(i).upper())
    counter+=1
print('---------------------------')
#Definitions for OLED-Display
WIDTH = 128
HIGHT = 64
oled = SSD1306_I2C(WIDTH, HIGHT, i2c)

#Definition for BME280
sensorBME = BME280(i2c=i2c)

i = 0
while True:
    valueTemp = sensor.read_u16() * conversion_factor #
    temp = 27 - (valueTemp - 0.706) / 0.001721 # See 4.9.5 of rp2040 datasheet
    tempC, preshPa, humRH = sensorBME.values #Receive current values from GY-BME280 as tuple
    tempC = tempC.replace('C','*C')
    print('Temperatur value: '+ '{:.2f}'.format(temp) +'*C') # Print in terminal with two decima
    print('Temperatur BME: ' + tempC)
    print('Pressure BME: ' + preshPa)
    print('Humidty BME: ' + humRH)
    print('Counter: ' + str(i))
    print('>-----------<')
    #Write data to display
    oled.fill(0)
    oled.text('GY-BME280 ',6,0)
    oled.text('Temp:' + tempC,6,14)
    oled.text('Pres:' + preshPa,6,28)
    oled.text('Humi:' + humRH,6,42)
    oled.text('Counter:' + str(i),6,56)
    oled.show()
    i+=1
    if i>999999:
        i=0
    utime.sleep(2) #Sleep for two seconds
    