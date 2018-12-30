# IRIGGenerator
IRIG-B timecode Signal Generator sample

## About IRIG-B time code
Refer this pages.
https://www.meinbergglobal.com/english/info/irig.htm
https://en.wikipedia.org/wiki/IRIG_timecode

## Support format
This sample sketch support B006. If you need other format, you can change it freely.

## Use Library and License
This sample sketch use follow libraries.
* Arduino DS3232RTC Library (https://github.com/JChristensen/DS3232RTC)
* Arduino Time Library (https://github.com/PaulStoffregen/Time)
* MsTimer2 Library

This program is distributed with GPL V3.

# Use Hardware
This sample sketch use this hardware.
* Arduino uno
* DS3231 RTC Module(like this module : https://www.amazon.co.jp/gp/product/B011DT3E36/)

# Hardware configuration
Connect the pins as follows:
 RTC          Arduino
 GND    ----  GND

VCC    ----  +5V

SCL    ----  SCL

SDA    ----  SDA

SQW    ----  D2
 
 
