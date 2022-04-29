# The Bust-A-Beat Soundboard
## About
The Bust-A-Beat Soundboard is an intuitive soundboard that can be used to create, play, and save simple songs made using an mBED LPC1768 development board for computing along with several other parts used for tasks from song display, saving/loading music, and, most importantly, playing some tunes.

This website documents the hardware and software used to create this project so that you might be able to make your own Bust-A-Beat. At the end of the documentation, it also displays some pictures and a video of what the final system should look and function like.

## Hardware
### Summary

|   Hardware Accessory   | Purpose                      |
|:-----------------------|:-----------------------------|
| mBED                   | Microcontroller for system   |
| Pushbuttons and LEDs   | Picking soundboard intrument |
| Touchpad Sensor        | Picking notes to play        |
| Color LCD Screen       | Displaying notes             |
| MicroSD Card System    | Saving and loading songs     |
| Speakers and Amplifier | Playing music                |
| Bluetooth Module       | Connecting with your phone   |
| External Power Supply  | Used to power accessories    |

### Hardware Accessories Further Explained
#### [Pushbuttons](https://os.mbed.com/users/4180_1/notebook/pushbuttons/) and LED's
3 pushbuttons are used in conjunction with 3 red LED's to choose and indicate the instrument (piano, synthesizer, and strings) the soundboard will emulate. 

Pushbuttons must have a Pullup, either internally or externally. We chose an internal software Pullup for our pushbuttons. Pushbuttons are then connected as PinDetect's for the mBed. On the other hand, LED's are wired in series with a resistor and connected to the mBed as a BusOut. The pin connections are summarized below.

| Instrument | Pushbutton Pin | LED Pin |
|:-----------|:---------------|:--------|
| Piano      | P19            | P22     |
| Synth      | P20            | P23     |
| Strings    | P16            | P24     |

#### [Touchpad Sensor](https://os.mbed.com/users/4180_1/notebook/mpr121-i2c-capacitive-touch-sensor/)
The touchpad sensor is used to navigate and select the notes and beats displayed on the LCD screen. 

The touchpad uses an I2C to communicate and is thusly wired with the mBED. The I@C interface uses 2 external Pullup resistors of 4.7 kOhms. In addition, one pin is used as an InterruptIn. The pin connections are summarized below.

|     mBED    | Touchpad | External Pullup Resistors |
|:------------|:---------|:--------------------------|
| GND         | GND      |                           |
| P28         | SDA      | P28 to Vout               |
| P27         | SCL      | P27 to Vout               |
| P26         | IRQ      |                           |
| Vout (3.3V) | Vcc      |                           |

#### [LCD Screen](https://os.mbed.com/users/4180_1/notebook/ulcd-144-g2-128-by-128-color-lcd/)
The LCD screen is used to display the individual notes in the song as it is being created, saved, loaded, and played. 

The LCD screen uses a Serial interface to communicate and connects to the mBED using a cable. and is also connected to the external supply for power. The pin connections using the cable and external power supply are summarized below.

| mBED | LCD Cable | External Supply |
|:-----|:----------|:----------------|
|      | 5V        | 5V              |
| GND  | GND       | GND             |
| P9   | TX        |                 |
| P10  | RX        |                 |
| P11  | Reset     |                 |

#### [MicroSD Card System](https://os.mbed.com/cookbook/SD-Card-File-System)
The microSD card system is used to save songs after they are constructed and to load songs that have been saved. Only one song can be saved to/loaded from memory at a time.

The breakout board used for communicating with the SD card system utlizes an SPI interface to communicate with the mBED. The pin connections are summarized below.

|    mBED     | SD Breakout Board |
|:------------|:------------------|
| P8          | CS                |
| P5          | DI                |
| Vout (3.3V) | Vcc               |
| P7          | SCK               |
| GND         | GND               |
| P6          | DO                |

#### [Speakers](https://os.mbed.com/users/4180_1/notebook/using-a-speaker-for-audio-output/) and [Amplifiers](https://www.sparkfun.com/products/11044)
The speakers are used to play the music that is being made. 

Both speakers connect to class D audio amplifiers. One speaker plays musical notes using AnalogOut while the other plays the beats using PWM. They both connect to the external power supply rather than the mBED Vout. Pin Connectiions are summarized below.

|           mBED           | Amplifier | Speaker| External Supply |
|:-------------------------|:----------|:-------|:----------------|
| GND                      | PWR-, IN- |        | GND             |
|                          | PWR+      |        | 5V              |
| P21 (Beats), P18 (Notes) | IN+       |        |                 |
|                          | OUT+      | +      |                 |
|                          | OUT-      | -      |                 |

