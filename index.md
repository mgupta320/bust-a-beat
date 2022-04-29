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

## Software

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

#### [Bluetooth Module](https://os.mbed.com/users/4180_1/notebook/adafruit-bluefruit-le-uart-friend---bluetooth-low-/)
The bluetooth module is used for getting directions from a users phone. The user can control volume, play/pause music, and save/load music from the SD card.

The bluetooth module communicates with the mBED using a RawSerial protocol. Pin connections are summarized below.

| mBED | BT Module | External Supply |
|:-----|:----------|:----------------|
| GND  | GND       | GND             |
|      | Vin       | 5V              |
| GND  | CTS       |                 |
| P14  | TXO       |                 |
| P13  | RXI       |                 |

## Software
Below, the changes made to mBED packages are listed and the entire code required to run the Bust-A-Beat Soundboard is written as well.

### Changes
#### Waveplayer
In order to allow for volume control, changes had to be made to the waveplayer package, primarily in the dac_out() function

```cpp
void wave_player::dac_out()
{
  if (DAC_on) {
#ifdef VERBOSE
  printf("ISR rdptr %d got %u\n",DAC_rptr,DAC_fifo[DAC_rptr]);
#endif
    wave_DAC->write_u16(DAC_fifo[DAC_rptr]*volume);
    DAC_rptr=(DAC_rptr+1) & 0xff;
  }
}
```
Changes were also made in other functions to add volume as an instance variable and functions to adjust the volume as necessary.

```cpp
wave_player::wave_player(AnalogOut *_dac)
{
  wave_DAC=_dac;
  wave_DAC->write_u16(32768);        //DAC is 0-3.3V, so idles at ~1.6V
  verbosity=0;
  volume = 0.5;
}
```
```cpp
void wave_player::play(FILE *wavefile, float _volume)
{
        unsigned chunk_id,chunk_size,channel;
        unsigned data,samp_int,i;
        short unsigned dac_data;
        long long slice_value;
        char *slice_buf;
        short *data_sptr;
        unsigned char *data_bptr;
        int *data_wptr;
        FMT_STRUCT wav_format;
        long slice,num_slices;
  volume = _volume;
 ...
```

### Main Code
The following is the main.cpp code used to run the Bust-A-Beat Soundboard

```cpp
#include "mbed.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "PinDetect.h"
#include "mpr121.h"
#include "rtos.h"
#include "wave_player.h"
#include <algorithm>
#include "stdio.h"
#include <string>
#include "SongPlayer.h"

//Volume level
BusOut myleds(LED1, LED2, LED3, LED4);

//Help guide recording if recording 
//If not recording then show notes being played
uLCD_4DGL uLCD(p9, p10, p11);

//Save recording and play recording from
SDFileSystem sd(p5, p6, p7, p8, "sd");

//Push button to switch instruments
PinDetect pianoPB(p19);
PinDetect synthPB(p20);
PinDetect stringsPB(p16);

//Red LEDs to show current instrument
BusOut redLEDs(p22, p23, p24);
//To show load or save
DigitalOut sdLED(p29);

// ======= INIT FOR TOUCH PAD =======
// Create the interrupt receiver object on pin 26
InterruptIn interrupt(p26);

// Setup the Serial to the PC for debugging
Serial pc(USBTX, USBRX);

// Setup the i2c bus on pins 28 and 27
I2C i2c(p28, p27);

// Setup the Mpr121:
// constructor(i2c object, i2c address of the mpr121)
Mpr121 mpr121(&i2c, Mpr121::ADD_VSS);

// Setup the first speaker for songplayer
SongPlayer mySpeaker(p21);

// Setup the second speaker for wave files
AnalogOut DACout(p18);
wave_player waver(&DACout);
// ======= INIT FOR TOUCH PAD =======

//Bluetooth
RawSerial bluemod(p13,p14);

//Threads
Thread thread2;
Thread thread4;
Thread thread5;

//Mutex Locks
//uLCD
Mutex stdio_mutex;
Mutex currState_mutex;
Mutex currInstr_mutex;

// ===== GLOBAL VARIABLES ======
enum instr{PIANO, SYNTH, STRINGS};
enum state{STARTING, IDLE, PLAYING, LOADING, SAVING};
enum playback{PAUSE, PLAY};
enum state2{S, L, R, U, D, P};

//Check how many places change state -> be sure to add mutex
string currInstr = "piano";
volatile int currState = STARTING;
volatile float volume = 0.5;
volatile int currPlayback = PAUSE;
volatile int nextNote = S;
volatile int currNotePos = 0;
volatile bool flash = false;

volatile int gridPosX = 0;
volatile int gridPosY = 0;

string notes[8] = {"0", "0", "0", "0", "0", "0", "0", "0"};
string beats[8] = {"0", "0", "0", "0", "0", "0", "0", "0"};
int raw_notes[8]={0,0,0,0,0,0,0,0};
int raw_beats[8]={0,0,0,0,0,0,0,0};
string files[8];

float note[1]= {100.0};
float duration[1]= {0.1};
// ===== GLOBAL VARIABLES ======

using namespace std;

void fallInterrupt() {
    int key_code=0;
    int i=0;
    int value=mpr121.read(0x00);
    value +=mpr121.read(0x01)<<8;
    for (i=0; i<12; i++) {
        if (((value>>i)&0x01)==1) key_code=i+1;
    }
    if ((key_code == 2) && (gridPosX < 7)) {
        //Right
        nextNote = R;
    } else if ((key_code == 7) && (gridPosY < 8)) {
        //Down
        nextNote = D;
    } else if (key_code == 6){
        nextNote = P;
    } else if (key_code == 5 && gridPosY > 0){
        //Up
        nextNote = U;
    } else if (key_code == 10 && gridPosX > 0) {
        //Left
        nextNote = L;
    } else {
        nextNote = S;
    }
}

void piano_hit_callback (void){  
    redLEDs.write(0x04);
    currInstr_mutex.lock();
        currInstr = "piano";
    currInstr_mutex.unlock();
}

void synth_hit_callback (void){  
    redLEDs.write(0x02);
    currInstr_mutex.lock();
        currInstr = "synth";
    currInstr_mutex.unlock();
}

void strings_hit_callback (void){  
    redLEDs.write(0x01);
    currInstr_mutex.lock();
        currInstr = "strings";
    currInstr_mutex.unlock();
}

void lcdThread() {
    while(1) {
        switch(currState) {
            case STARTING:
                for (int x = 0; x < 128; x = x + 16)
                {
                    for (int y = 0; y < 128; y = y + 14)
                    {
                        int currX1 = x;
                        int currX2 = x + 16;
                        int currY1 = y;
                        int currY2 = y + 14;
                        stdio_mutex.lock();
                        uLCD.rectangle(currX1, currY1, currX2, currY2, WHITE);
                        uLCD.filled_rectangle((currX1 + 1), (currY1+1), (currX2-1), (currY2-1), LGREY);
                        stdio_mutex.unlock();
                    }
                }
                stdio_mutex.lock();
                uLCD.line(0, (14*8), 127, (14*8), BLACK);
                stdio_mutex.unlock();
                currState_mutex.lock();
                currState = IDLE;
                currState_mutex.unlock();
                break;
            case IDLE:
                int currX1 = gridPosX * 16 + 1;
                int currX2 = (gridPosX + 1) * 16 - 1;
                int currY1 = gridPosY * 14 + 1;
                int currY2 = (gridPosY + 1) * 14 - 1;
                int nextX1, nextX2, nextY1, nextY2;
                int newGridPosX = gridPosX;
                int newGridPosY = gridPosY;
                switch (nextNote){
                    case S:  
                        nextX1 = currX1;
                        nextX2 = currX2;
                        nextY1 = currY1;
                        nextY2 = currY2;
                        break;
                    case P:  
                        nextX1 = currX1;
                        nextX2 = currX2;
                        nextY1 = currY1;
                        nextY2 = currY2;
                        break;
                    case U:
                        nextX1 = currX1;
                        nextX2 = currX2;
                        nextY1 = currY1 - 14;
                        nextY2 = currY2 - 14;
                        newGridPosY = gridPosY - 1;
                        break;
                    case D:
                        nextX1 = currX1;
                        nextX2 = currX2;
                        nextY1 = currY1 + 14;
                        nextY2 = currY2 + 14;
                        newGridPosY = gridPosY + 1;
                        break;
                    case L:
                        nextX1 = currX1 - 16;
                        nextX2 = currX2 - 16;
                        nextY1 = currY1;
                        nextY2 = currY2;
                        newGridPosX = gridPosX - 1;
                        break;
                    case R:
                        nextX1 = currX1 + 16;
                        nextX2 = currX2 + 16;
                        nextY1 = currY1;
                        nextY2 = currY2;
                        newGridPosX = gridPosX + 1;
                        break;
                        
                }
                int currColor = LGREY;
                if (nextNote == P) {
                    if (gridPosY != 8) {
                        if (raw_notes[gridPosX] == gridPosY + 1) {
                            raw_notes[gridPosX] = 0;
                        } else if (raw_notes[gridPosX] != 0) {
                            int prevY1 = (raw_notes[gridPosX] - 1) * 14 + 1;
                            int prevY2 = raw_notes[gridPosX] * 14 - 1;
                            uLCD.filled_rectangle(currX1, prevY1, currX2, prevY2, LGREY);
                            currColor = RED;
                            raw_notes[gridPosX] = gridPosY + 1;
                        } else {
                            currColor = RED;
                            raw_notes[gridPosX] = gridPosY + 1;
                        }
                    } else {
                        raw_beats[gridPosX] = (raw_beats[gridPosX] + 1) % 2;
                        if (raw_beats[gridPosX] == 1) {
                           currColor = RED;
                        }
                    } 
                } else if ((raw_notes[gridPosX] == gridPosY + 1) || (gridPosY == 8 && raw_beats[gridPosX] == 1))  {
                    currColor = RED;
                }
                gridPosX = newGridPosX;
                gridPosY = newGridPosY;        
                
                stdio_mutex.lock();
                uLCD.filled_rectangle(currX1, currY1, currX2, currY2, currColor);
                if (flash) {
                    uLCD.filled_rectangle(nextX1, nextY1, nextX2, nextY2, DGREY);                    
                }
                stdio_mutex.unlock();
                flash = !flash;
                nextNote = S;
                break;
            case PLAYING:
                switch(currPlayback){
                    int currX1, currX2, prevX1, prevX2;
                    case PLAY:
                        if (currNotePos == 0) {
                            prevX1 = 112;
                            prevX2 = 128;
                        } else {
                            prevX1 = (currNotePos - 1) * 16;
                            prevX2 = currNotePos * 16;
                        }
                        currX1 = currNotePos * 16;
                        currX2 = (currNotePos + 1) * 16;
                        stdio_mutex.lock();
                        uLCD.rectangle(prevX1, 0, prevX2, 126, WHITE);
                        uLCD.line(0, (14*8), 127, (14*8), BLACK);
                        uLCD.rectangle(currX1, 0, currX2, 126, BLUE);
                        stdio_mutex.unlock();
                        break;
            
                    case PAUSE:
                        currX1 = currNotePos * 16;
                        currX2 = (currNotePos + 1) * 16;
                        currNotePos = 0;
                        stdio_mutex.lock();
                        uLCD.rectangle(currX1, 0, currX2, 126, WHITE);
                        uLCD.line(0, (14*8), 127, (14*8), BLACK);
                        stdio_mutex.unlock();
                        currState_mutex.lock();
                        currState = IDLE;
                        currState_mutex.unlock();
                        break;
                }
                break;
            case LOADING:
                for (int small_x = 0; small_x < 8; small_x++)
                {
                    int filled_note_row = raw_notes[small_x];
                    int filled_beat = raw_beats[small_x];
                    int x = small_x * 16; 
                    for (int small_y = 0; small_y < 9; small_y++)
                    {
                        int currColor = LGREY;
                        int y = small_y * 14; 
                        if ((small_y + 1 == filled_note_row) || (small_y == 8  && filled_beat)) {
                            currColor = RED;
                        } 
                        int currX1 = x;
                        int currX2 = x + 16;
                        int currY1 = y;
                        int currY2 = y + 14;
                        
                        stdio_mutex.lock();
                        uLCD.rectangle(currX1, currY1, currX2, currY2, WHITE);
                        uLCD.filled_rectangle((currX1 + 1), (currY1+1), (currX2-1), (currY2-1), currColor);
                        stdio_mutex.unlock();
                    }
                }
                stdio_mutex.lock();
                uLCD.line(0, (14*8), 127, (14*8), BLACK);
                stdio_mutex.unlock();
                currState_mutex.lock();
                currState = IDLE;
                currState_mutex.unlock();
                break;
            case SAVING:
                for (int x = 0; x < 128; x = x + 16)
                {
                    for (int y = 0; y < 128; y = y + 14)
                    {
                        int currX1 = x;
                        int currX2 = x + 16;
                        int currY1 = y;
                        int currY2 = y + 14;
                        stdio_mutex.lock();
                        uLCD.rectangle(currX1, currY1, currX2, currY2, RED);
                        stdio_mutex.unlock();
                    }
                }
                for (int x = 0; x < 128; x = x + 16)
                {
                    for (int y = 0; y < 128; y = y + 14)
                    {
                        int currX1 = x;
                        int currX2 = x + 16;
                        int currY1 = y;
                        int currY2 = y + 14;
                        stdio_mutex.lock();
                        uLCD.rectangle(currX1, currY1, currX2, currY2, WHITE);
                        stdio_mutex.unlock();
                    }
                }
                stdio_mutex.lock();
                uLCD.line(0, (14*8), 127, (14*8), BLACK);
                stdio_mutex.unlock();
                currState_mutex.lock();
                currState = IDLE;
                currState_mutex.unlock();
                break;
        }
        Thread::wait(1000.0*0.2);
    }
    
}

void noteThread() {
    while(1) {
        if (currState == PLAYING) {
            FILE * wav_file; //increase PWM clock rate for audio
            while(currState == PLAYING && currPlayback == PLAY) {
                for (int i = 0; i < 8 && currPlayback == PLAY; i++) {
                    currNotePos = i;
                    char curr_beat[1];
                    int n = sprintf(curr_beat, "%d", raw_beats[i]);
                    char curr_note[1];
                    int m = sprintf(curr_note, "%d", raw_notes[i]);
                    if (curr_beat == "1") {
                        mySpeaker.PlaySong(note,duration,volume/500.0);
                    }
                    currInstr_mutex.lock();
                    if (raw_notes[i] != 0) {
                        files[i] = "/sd/BABNotes/" + currInstr + "/" + currInstr + "-0" + curr_note + ".wav";
                        char * tab2 = new char [files[i].length()+1];
                        strcpy (tab2, files[i].c_str());
                        wav_file = fopen(tab2, "r");
                        waver.play(wav_file, volume);
                        fclose(wav_file);
                    } else {
                        files[i] = "/sd/BABNotes/" + currInstr + "/" + currInstr + "-0" + "1" + ".wav";
                        char * tab2 = new char [files[i].length()+1];
                        strcpy (tab2, files[i].c_str());
                        wav_file = fopen(tab2, "r");
                        waver.play(wav_file, 0);
                        fclose(wav_file);
                    }
                    currInstr_mutex.unlock();
                }
            }
        }
    Thread::wait(1000.0*0.2);
    }
}


void blueToothThread() {
    char bnum=0;
    char bhit=0;
    while(1) {
        if (bluemod.getc()=='!') {
            if (bluemod.getc()=='B') { //color data packet
                bnum = bluemod.getc(); //button number
                bhit = bluemod.getc(); //1=hit, 0=release
                if (bluemod.getc()==char(~('!' + 'B' + bnum + bhit))) {
                    
                    switch (bnum) {
                        case '1': //Playback Start
                        {   currState_mutex.lock();
                            currPlayback = PLAY;
                            currState_mutex.unlock();
                            break;   
                        }
                        case '2': //Playback Stop
                        {
                            currState_mutex.lock();
                            currPlayback = PAUSE;
                            currState_mutex.unlock();
                            break;
                        }
                        case '3': //Save recording
                        {
                            FILE *fp = fopen("/sd/BABNotes/song.txt", "w"); //open SD
                            if(fp == NULL) {
                                error("Could not open file for write\n");
                            }
                            //fprintf(fp, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                            for (int i = 0; i < 8; i++) {
                                fprintf(fp, "%d ", raw_notes[i]); // write SD
                            }
                            for (int i = 0; i < 8; i++) {
                                fprintf(fp, "%d ", raw_beats[i]); // write SD
                            }
                            fclose(fp);
                            currState_mutex.lock();
                            currState = SAVING;
                            currState_mutex.unlock();
                            break;
                        }
                        case '4': //Load recording
                        {
                            string inputString;
                            FILE *fp = fopen("/sd/BABNotes/song.txt", "r");
                            if(fp == NULL) {
                            } else {
                                int i = 0;
                                while (fscanf(fp,"%s", inputString)!= EOF) //reads in a string delineated by white space
                                { 
                                    if (i < 8) {
                                        notes[i] = inputString.c_str();
                                        raw_notes[i] = notes[i].c_str()[0] - '0';
                                    } else if (i < 15) {
                                        beats[i-8] = inputString.c_str();
                                        raw_beats[i-8] = beats[i-8].c_str()[0] - '0';
                                    } 
                                    i++;
                                }
                                currState_mutex.lock();
                                currState = LOADING;
                                currState_mutex.unlock();
                            }
                            fclose(fp);
                            break;
                        }
                        case '5': //Volume Up
                        {
                            volume = std::min(1.0, volume + 0.05);
                            break;
                        }
                        case '6': //Volume Down
                        {
                            volume = std::max(0.0, volume - 0.05);
                            break;
                        }
                        default:
                            break;
                    }                    
                }
            }
        }
        if (volume >= 1.0) {
            myleds.write(0x0F);    
        } else if (volume >= 0.75) {
            myleds.write(0x07);    
        } else if (volume >= 0.50) {
            myleds.write(0x03); 
        } else if (volume >= 0.25) {
            myleds.write(0x01); 
        } else {
            myleds.write(0x00);
        }  
    }
}

void init() {
    pianoPB.mode(PullUp);
    synthPB.mode(PullUp); 
    stringsPB.mode(PullUp); 
    interrupt.mode(PullUp);
    wait(0.01);

    pianoPB.attach_deasserted(&piano_hit_callback);
    synthPB.attach_deasserted(&synth_hit_callback);
    stringsPB.attach_deasserted(&strings_hit_callback);
    interrupt.fall(&fallInterrupt);
    
    pianoPB.setSampleFrequency();
    synthPB.setSampleFrequency();
    stringsPB.setSampleFrequency();
    
    redLEDs.write(0x04);
    
    thread2.start(lcdThread);
    thread4.start(noteThread);
    thread5.start(blueToothThread);
}

int main() {
    init();
    wait(2);
    currState = STARTING;
    while(1) {
        if (currState == LOADING || currState == SAVING) sdLED = 1;
        else sdLED = 0;
        if (currState == IDLE && currPlayback == PLAY){ 
            currState_mutex.lock();
            currState = PLAYING;
            currState_mutex.unlock();
        }
        Thread::wait(1000.0*0.2);
    }
}
```
## Examples and Demonstration
### Examples
The following are pictures of the individual hardware accessories and how they should appear after being wired and connected to the mBED.
![LCD Screen](/assets/images/LCD.jpg)
