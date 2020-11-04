# ESP8266 Ultrasonic People Counter
Simple entry counter design for church entry counting during COVID-19 times.

![alt tag](https://github.com/BubuHub/ESP8266UltrasonicPeopleCounter/blob/main/blob/assets/outside.jpeg)

# Parts needed:
* ESP8266
* US-100 ultrasonic sensor
* TFT ST7735 SPI based display (160x128) 1.8''
* Rotary encoder + switch like KY-040
* Active mini piezo buzzer (3.3V generator inside)
* 18650 ACU like NCR18650B
* USB charger/protection board like TP4056
* Low Drop 3.3V LDO
* power switch
* case
* wires
* hot glue

# ESP8266 CONNECTIONS
* TFT_CS     - GPIO16 (CS)
* TFT_SCK    - GPIO14 (SCL)
* TFT_RS     - GPIO12 (A0)
* TFT_MOSI   - GPIO13 (SDA)
* TFT_RESET  - GPIO15 (RESET)
* TFT_LED+   - 100ohm -> 3.3V
* ROT1       - GPIO5 - filter (ESP pin - 10kohm (pullup) - 100nF to GND - 1kohm to rotary)
* ROT2       - GPIO4 - the same filter
* BTN        - GPIO0 - the same filter
* LED/BUZZER - GPIO2 (Buzzer connected to 3.3V GPIO2 drives to GND)
* VBAT - 20kohm - A0 - 4.7kohm - GND
                     |- 100nF -|

# Wiring

![alt tag](https://github.com/BubuHub/ESP8266UltrasonicPeopleCounter/blob/main/blob/assets/schematic.png)
![alt tag](https://github.com/BubuHub/ESP8266UltrasonicPeopleCounter/blob/main/blob/assets/inside.jpeg)

# Building under Linux
* install PlatformIO
* enter project directory
* connect PC computer over USB to 3.3V TTL serial converter to ESP8266 GND, rx and tx pin
* Press button on rotary and power up the device to enter programming mode
* type in terminal:
  platformio run -t upload

You can also use IDE to build this project on Linux/Windows/Mac. My fvorite ones:
* [Code](https://code.visualstudio.com/)
* [Atom](https://atom.io/)
