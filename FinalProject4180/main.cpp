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
    pc.printf("in lcd thread\n");
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
    pc.printf("in note thread\n");
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
                    pc.printf(curr_note);
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
    pc.printf("inside bluetooth thread \n");
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
                            
                            for (int i = 0; i < 8; i++) {
                                pc.printf(notes[i].c_str());
                            }
                            pc.printf("\n");
                            for (int i = 0; i < 8; i++) {
                                pc.printf(beats[i].c_str());
                            }
                            pc.printf("\n");
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
        if (currState == IDLE && currPlayback == PLAY) currState = PLAYING;
        Thread::wait(1000.0*0.2);
    }
}
