# The Bust-A-Beat Soundboard
## About
The Bust-A-Beat Soundboard is an intuitive soundboard that can be used to create, play, and save simple songs made using an mBED LPC1768 development board for computing along with several other parts used for tasks from song display, saving/loading music, and, most importantly, playing some tunes.

This website documents the hardware and software used to create this project so that you might be able to make your own Bust-A-Beat.

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

