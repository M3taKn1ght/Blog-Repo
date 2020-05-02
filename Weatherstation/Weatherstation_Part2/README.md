# Weatherstation part 2
Next step for our weatherstation. First we want to know the weather data for our location and want to display them
on a simple OLED-display. Every 5 minutes we want to have the newest weatherforecast and every minute an update to our output.
Also we want to know sunrise and sunset, also moonphase is interessting. Sunrise and sunset will extract from weatherforecast,
moonphase will calculated by hand.

1. [ESP32 NodeMCU](https://www.az-delivery.de/products/esp32-developmentboard?_pos=24&_sid=69909e42c&_ss=r)
2. [0.96" OLED i2c Display ](https://www.az-delivery.de/products/0-96zolldisplay?_pos=14&_sid=03d542ee1&_ss=r)
3. [Jumper wire cable female to male](https://www.az-delivery.de/products/40-stk-jumper-wire-female-to-male-20-zentimeter?_pos=20&_sid=5cfea44cd&_ss=r)
4. An account from [Openweathermap.org](https://openweathermap.org/)

After connecting like shown below

![Schema](images/Weatherstation_Part1.jpg)

and adding the following libaries
1. NTPClient
2. WiFi
3. WiFiClientSecure.h
4. Adafruit SSD1306
5. ArduinoJson
6. Time

and uploading the ino-File (**after modifing the TODOs**) to NodeMCU, the result looks like

![LiveView](images/LiveView.jpg)
