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

### Pin Connections for Hardware Accessories
#### [Pushbuttons](https://os.mbed.com/users/4180_1/notebook/pushbuttons/) and LED's
3 pushbuttons are used in conjunction with 3 red LED's to choose and indicate the instrument (piano, synthesizer, and strings) the soundboard will emulate. 

Pushbuttons must have a Pullup, either internally or externally. We chose an internal software Pullup for our pushbuttons. Pushbuttons are then connected as DigitalIn's for the mBed. On the other hand, LED's are wired in series with a resistor and connected to the mBed as a BusOut. 

| Instrument | Pushbutton Pin | LED Pin |
|:-----------|:---------------|:--------|
| Piano      | P19            | P22     |
| Synth      | P20            | P23     |
| Strings    | P16            | P24     |

