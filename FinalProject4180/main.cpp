#include "mbed.h"
#include "Speaker.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "PinDetect.h"
#include "mpr121.h"
#include "rtos.h"
#include "wave_player.h"
#include <algorithm>

//Volume level
BusOut myleds(LED1, LED2, LED3, LED4);

//Frequency one of song
Speaker speaker1(p21);
//Frequency two of song
Speaker speaker2(p21);

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
// ======= INIT FOR TOUCH PAD =======

//Bluetooth
RawSerial bluemod(p13,p14);

//Mutex Locks
//uLCD
Mutex stdio_mutex;
Mutex currState_mutex;

// ===== GLOBAL VARIABLES ======
enum instr{PIANO, SYNTH, STRINGS};
enum state{STARTING, IDLE, PLAYING, LOADING, SAVING};
enum playback{PAUSE, PLAY};

//Check how many places change state -> be sure to add mutex
volatile int currInstr = PIANO;
volatile int currState = STARTING;
volatile int volume = 0.5;
volatile int playback = PLAY;

volatile int gridPosX = 0;
volatile int gridPosY = 0;
// ===== GLOBAL VARIABLES ======

void fallInterrupt() {
    int key_code=0;
    int i=0;
    int value=mpr121.read(0x00);
    value +=mpr121.read(0x01)<<8;
    for (i=0; i<12; i++) {
        if (((value>>i)&0x01)==1) key_code=i+1;
    }
    switch (key_code) {
        case 1: //Right
            gridPosX = std::min(8, gridPosX + 1);
            break;
        case 4: //Up
            gridPosY = std::max(8, gridPosY + 1);
            break;
        case 5: //Select
            break;
        case 6: //Down
            gridPosY = std::min(0, gridPosY - 1);
            break;
        case 9: //Left
            gridPosX = std::min(0, gridPosX - 1);
            break;
        default:
            break;
    }
}

void piano_hit_callback (void){  
    redLEDs.write(0x04);
    currInstr = PIANO;
}

void synth_hit_callback (void){  
    redLEDs.write(0x02);
    currInstr = SYNTH;
}

void strings_hit_callback (void){  
    redLEDs.write(0x01);
    currInstr = STRINGS;
}

void lcdThread(void const *args) {
    while(1) {
//        switch(currState) {
//            case STARTING:
//                stdio_mutex.lock();
//                    uLCD.text_width(4);
//                    uLCD.text_height(4);
//                    uLCD.printf("Welcome to\n");
//                    uLCD.printf("BUST A BEAT\n");
//                stdio_mutex.unlock();
//                break;
//            case IDLE:
//                break;
//            case PLAYING:
//                break;
//            case LOADING:
//                break;
//            case SAVING:
//                break;
//        }
        Thread::wait(1000.0*0.2);
    }
}

void beatThread(void const *args) {
    while(1) {
//        if (currState == PLAYING) {
//            
//        }
        Thread::wait(1000.0*0.2);
    }
}

void noteThread(void const *args) {
    while(1) {
        if (currState == PLAYING) {
            
        }
        Thread::wait(1000.0*0.2);
    }
}

void blueToothThread(void const *args) {
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
                            playback = PLAY;
                            break;   
                        case '2': //Playback Stop
                            playback = PAUSE;
                            break;
                        case '3': //Save recording
                            currState = SAVING;
                            break;
                        case '4': //Load recording
                            currState = LOADING;
                            break;
                        case '5': //Volume Up
                            volume = std::min(1.0, volume + 0.1);
                            break;
                        case '6': //Volume Down
                            volume = std::max(0.0, volume - 0.1);
                            break;
                        default:
                            break;
                    }                    
                }
            }
        }
        if (volume >= 1) {
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
        myleds.write(0x0F);  
        Thread::wait(1000.0*0.2);
    }
}

void init() {
    pianoPB.mode(PullUp);
    synthPB.mode(PullUp); 
    stringsPB.mode(PullUp); 
    interrupt.mode(PullUp);
    wait(0.01);
    pc.printf("I am here\n");
    pianoPB.attach_deasserted(&piano_hit_callback);
    synthPB.attach_deasserted(&synth_hit_callback);
    stringsPB.attach_deasserted(&strings_hit_callback);
    interrupt.fall(&fallInterrupt);
    
    pianoPB.setSampleFrequency();
    synthPB.setSampleFrequency();
    stringsPB.setSampleFrequency();
    
    Thread thread2(lcdThread);
    Thread thread3(beatThread);
    Thread thread4(noteThread);
    Thread thread5(blueToothThread);
}

int main() {
    init();
    //Touchpad
    currState = IDLE;
    while(1) {
        wait(0.2);
    }
}