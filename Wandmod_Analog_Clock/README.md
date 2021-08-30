# Analog clock with AZ-Touch Mod 
![Schema](images/9_Title.jpg)

## Description
A small anlaoge clock on AZ touch wall mod with sunrise and sunset display. At least one basic account at openweathermap.org is needed to synchronize the data. The data is then displayed accordingly on the 2.4" or 2.8" display.

## Code
Copy User_Setup.h.new in your TFT_eSPI-Path and rename it to User_Setup.h (backup the old one). 
In the source code the WiFi access data, the openweathermap.org API key and the location must be entered, otherwise this example will not work.