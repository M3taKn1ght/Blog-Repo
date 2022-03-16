"""
// Read internal temp-sensor
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 23. Oct 2021
// Update:  23. Oct 2021
"""

 #First import needed libs
import machine #Important to get Pins and config them
import utime #For a little timer

sensor = machine.ADC(4) # Create object sensor and init as pin ADC(4)
conversion_factor = 3.3 / 65535 # 3.3V are 16bit (65535)

while True:
    valueTemp = sensor.read_u16() * conversion_factor #
    temp = 27 - (valueTemp - 0.706) / 0.001721 # See 4.9.5 of rp2040 datasheet
    print('Temperatur value: '+ "{:.2f}".format(temp)) # Print in terminal with two decimal
    utime.sleep(2) #Sleep for two seconds