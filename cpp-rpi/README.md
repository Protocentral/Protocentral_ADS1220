# ADS1220

### C++ example (for Raspberry Pi)

##### Prerequisites
- Install [WiringPi](http://wiringpi.com);
- Install [spidev-lib++](https://github.com/milekium/spidev-lib).

##### Wiring

All pins numbers are referring to the physical number.

| Raspberry Pi   | ADS1220 breakout | Note 
|----------------|------------------|-
| Pin 13         | DRDY             | Or other GPIO pin
| Pin 21 (MISO)  | MISO             |
| Pin 19 (MOSI)  | MOSI             |
| Pin 23 (SCLK)  | SCLK             |
| Pin 18         | CS               | Or other GPIO pin
| GND            | CLK              |
| 3.3V           | DVDD             |
| GND            | DGND             |
| 3.3V           | AVDD             |
| GND            | AGND             |
| N/A            | REFN0 / REFP0    | Floating
| N/A            | AIN0-3           | Connect your inputs (e.g. pot)

##### Compile & run

```
g++ -std=c++20 -o ads example.cpp ads1220.cpp -lspidev-lib++ -lwiringPi
./ads
```