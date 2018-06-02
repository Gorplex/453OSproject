#include <avr/io.h>
#include <avr/interrupt.h>
//#include "globals.h"
#include "serial.h"
#include "os.h"
#include <util/delay.h>
#include <stdlib.h>
#include "ext2.h"
#include "SdReader.h"
#include "printThreads.h"

#define BUF_SIZE 256


typedef struct music_t {
   uint16_t playI;
   uint16_t readI;
   uint8_t buf1[BUF_SIZE];
   uint8_t buf2[BUF_SIZE];
} music_t;

void initMusic(music_t *m){
   m->playI=0;
   m->readI=0;
}

void printThreadsMain(uint16_t *sys){
   while(1){
      printSys((system_t *) sys);
      yield();
   }
}

void idle(uint16_t *args){
   while(1){

   }
}

void playbackMain(music_t *music){
   while(1){
      
   }
}

void readMain(){
   sdInit(0);
   while(1){
      if(1){

      }
      yield();
   }
}

int main(void) {
   uint8_t sd_card_status;
   system_t * sys;
   music_t music;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 
   clear_screen();

   start_audio_pwm();
   sys = os_init_noMain();
   
   create_thread("playback", (uint16_t) &playbackMain, &music, 5);
   create_thread("reader", (uint16_t) &readMain, &music, 1000);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   create_thread("idle", (uint16_t) &idle, NULL, 5);


   return 0;
}
