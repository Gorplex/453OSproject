#include <avr/io.h>
#include <avr/interrupt.h>
//#include "globals.h"
#include "serial.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
//#include "ext2.h"
#include "SdReader.h"
#include "printThreads.h"
#include "ext2Reader.h"

#define BUF_SIZE 256
#define NAME_LEN 256

system_t *sys;

typedef struct music_t {
   uint16_t playI;
   uint16_t readI;
   uint16_t songNum;
   uint32_t size;
   uint16_t bufNum;
   uint8_t buf[BUF_SIZE];
   uint8_t buf2[BUF_SIZE];
   uint8_t name[NAME_LEN];
} music_t;

void initMusic(music_t *m){
   m->playI=0;
   m->readI=0;
   m->songNum = 0;
   m->bufNum = 0;
   m->size = 0;
   //bug may not skip back over dirs properly
}

void printThreadsMain(music_t *music){
   while(1){
      printSys((system_t *) sys);
      yield();
   }
}

void idle(uint16_t *args){
   while(1){

   }
}

//check
void playbackMain(music_t *music){
   while(1){
      if(music->readI/BUF_SIZE ^ music->playI/BUF_SIZE){
         OCR2B = music->buf[music->playI++];
         if(music->playI >= 512){
            music->playI = 0;
         }
      }
      thread_sleep(1);   //sleep for 45us (1s/22000)
   }
}

void readMain(music_t *music){
   uint16_t fileIndex;
   uint16_t lastSongNum;
   uint32_t inodeNum;

   sdInit(0);
   fileIndex = 0;
   lastSongNum = music->songNum; 
   
   while(1){
      //first read
      inodeNum = readRoot(&fileIndex, music->name, &music->size);
      readFile(inodeNum, music->bufNum, music->buf); 
      music->bufNum++;
      music->readI=BUF_SIZE;

      while(1){
         //end of song load next
         if(music->bufNum == 0){
            lastSongNum++;
            music->songNum++;
            break;
         }
         
         if(music->songNum != lastSongNum){
            if(music->songNum > lastSongNum){
               fileIndex++;
            }else{ //if(music->songNum < lastSongNum){
               fileIndex--;   //may be a problem
            }
            lastSongNum = music->songNum;         
            break;
         }
         //if queue needs to be filled
         if(music->readI/BUF_SIZE ^ music->readI/BUF_SIZE){
            readFile(inodeNum, music->bufNum, music->buf); 
            music->bufNum++;
            music->readI = (music->readI+BUF_SIZE)%(BUF_SIZE*2);
         }
         yield();
      }
      //wait for player to be in second buffer
      while(!(music->playI/BUF_SIZE)){
         yield();
      }
   }
}

void main() {
   uint8_t sd_card_status;
   music_t music;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   initMusic(&music);

   serial_init(); 
   clear_screen();

   start_audio_pwm();
   sys = os_init_noMain();
   
   create_thread("playback", (uint16_t) &playbackMain, &music, 5);
   create_thread("reader", (uint16_t) &readMain, &music, 1000);
   create_thread("stats", (uint16_t) &printThreadsMain, &music, PRINT_THREAD_SIZE);
   create_thread("idle", (uint16_t) &idle, NULL, 5);

   os_start();
}

