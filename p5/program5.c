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


#define BAR_LEN 50

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
      /* if not in same buffer (aka, wait till 1st buf read) */
      if(music->readI/BUF_SIZE != music->playI/BUF_SIZE){
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
   uint32_t inodeNum;

   /* sdInit(0); */
   fileIndex = 0;
   
   while(1){
      //first read
      inodeNum = readRoot(&fileIndex, music->name, &music->size);
      readFile(inodeNum, music->bufNum, music->buf); 
      music->bufNum++;
      music->readI=BUF_SIZE;

      while(1){
         //end of song load next
         if(music->bufNum * BUF_SIZE >= music->size){
            music->songNum++;
            break;
         }

	 if(byte_available()) {
	    uint8_t c = read_byte();
	    if(c == 'p') {
	       fileIndex--;
	       music->songNum--;
	       break;
	    } else if( c == 'n') {
	       fileIndex++;
	       music->songNum++;
	       break;
	    }
	 }
	    
	 	 
         //if queue needs to be filled
         if(music->readI/BUF_SIZE != music->readI/BUF_SIZE){
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



void printMusic(music_t * m) {
   char bar[50];

   set_cursor(33,30);
   set_color(GREEN);
   int i;
   for(i=0; i < BAR_LEN * (m->bufNum / 86 /* 85.9375 */) / m->size/22000; i++) {
      write_byte(219); /* '█'); */
   }
   set_color(YELLOW);
   while(i < 50) {
      write_byte('_');
   }

      

   set_cursor(30,50);
   set_color(GREEN);
   print_string("\r\n\tSong Time:\t"); 
   print_int( m->bufNum / 86 /* 85.9375 */);

   
   
   set_cursor(30,50);
   set_color(YELLOW);
   print_string("\r\n\tSong Time:\t"); 
   print_int( m->bufNum / 86 /* 85.9375 */);

   set_cursor(31,50);
   set_color(GREEN);
   print_string("\r\n\tSong Length:\t"); 
   print_int( m->size / 22000);

   set_cursor(32,50);
   set_color(GREEN);
   print_string("\r\n\tSong Num:\t"); 
   print_int( m->songNum);

}
