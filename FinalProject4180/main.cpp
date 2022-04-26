#include "mbed.h"
#include "Speaker.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "PinDetect.h"
#include "mpr121.h"
#include "rtos.h"
#include "wave_player.h"

//Volume level
BusOut myleds(LED1, LED2, LED3, LED4);

//Frequency one of song
Speaker speaker1(p21);
//Frequency two of song
Speaker speaker2(p21);

//Help guide recording if recording 
//If not recording then show notes being played
uLCD_4DGL uLCD(p28, p27, p29);

//Save recording and play recording from
SDFileSystem sd(p5, p6, p7, p8, "sd");

//Push button to switch instruments
PinDetect pianoPB(p5);
PinDetect synthPB(p6);
PinDetect stringsPB(p7);

//Red LEDs to show current instrument
BusOut redLEDs(p5, p6, p7);

// ======= INIT FOR TOUCH PAD =======
// Create the interrupt receiver object on pin 26
InterruptIn interrupt(p26);

// Setup the Serial to the PC for debugging
Serial pc(USBTX, USBRX);

// Setup the i2c bus on pins 28 and 27
I2C i2c(p9, p10);

// Setup the Mpr121:
// constructor(i2c object, i2c address of the mpr121)
Mpr121 mpr121(&i2c, Mpr121::ADD_VSS);
// ======= INIT FOR TOUCH PAD =======

//Bluetooth
Serial bluemod(p13,p14);

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
// ===== GLOBAL VARIABLES ======

void piano_hit_callback (void){  
    myleds.write(0x04);
    currInstr = PIANO;
}

void synth_hit_callback (void){  
    myleds.write(0x02);
    currInstr = SYNTH;
}

void strings_hit_callback (void){  
    myleds.write(0x01);
    currInstr = STRINGS;
}

void lcdThread(void const *args) {
    while(1) {
        switch(currState) {
            case STARTING:
                break;
            case IDLE:
                break;
            case PLAYING:
                break;
            case LOADING:
                break;
            case SAVING:
                break;
        }
        Thread::wait(1000.0*0.2);
    }
}

void beatThread(void const *args) {
    while(1) {
        if (currState == PLAYING) {
            
        }
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
                            volume += 0.1;
                            break;
                        case '6': //Volume Down
                            volume -= 0.1;
                            break;
                        case '7': //Number 3
                            break;
                        case '8': //Number 3
                            break;
                    }
                }
            }
        }
        Thread::wait(1000.0*0.2);
    }
}

void init() {
    pianoPB.mode(PullUp);
    synthPB.mode(PullUp); 
    stringsPB.mode(PullUp); 
    wait(0.1);
    
    pianoPB.attach_deasserted(&piano_hit_callback);
    synthPB.attach_deasserted(&synth_hit_callback);
    stringsPB.attach_deasserted(&strings_hit_callback);
    
    Thread thread2(lcdThread);
    Thread thread3(beatThread);
    Thread thread4(noteThread);
    Thread thread5(blueToothThread);
}

void welcome() {
    stdio_mutex.lock();  
    stdio_mutex.unlock();
    Thread::wait(1000.0*0.2);    
}

int main() {
    init();
    welcome();
    while(1) {
        
    };
}