# CVURobohawksArduinoVehicle

- CVURobohawksVehicleControlHTML.html and CVURobohawksVehicleJoyStick.js based on code from https://automatedhome.party/html-and-js-code-for-wifi-car/

Dependencies:
- Arduino/ESP8266 Servo.h library
- CaileanSorce_nodemcuESP8266_pinDefinitions.h ; contains the following definitions:
```
    static const uint8_t D0 = 16;
    static const uint8_t D1 = 5;
    static const uint8_t D2 = 4;
    static const uint8_t D3 = 0;
    static const uint8_t D4 = 2;
    static const uint8_t D5 = 14;
    static const uint8_t D6 = 12;
    static const uint8_t D7 = 13;
    static const uint8_t D8 = 15;
```
    
See CVURobohawksVehicleMain.ino for an example of how to use the library