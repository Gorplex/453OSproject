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


/* music display stuff */
#define BAR_LEN 50
#define MUSIC_X 25
#define MUSIC_Y 37
system_t *sys;

char debug[512];

typedef struct music_t {
   uint16_t playI;
   uint16_t readI;
   uint16_t songNum;
   uint32_t size;
   uint16_t bufNum;
   uint8_t buf[BUF_SIZE];
   uint8_t buf2[BUF_SIZE];
   char name[NAME_LEN];
} music_t;

void initMusic(music_t *m){
   memset(m, 0, sizeof( music_t));
   strcpy(m->name, "no title");
   //bug may not skip back over dirs properly
}

void printMusic(music_t * m) {
   set_cursor(MUSIC_Y,MUSIC_X);
   set_color(GREEN);
   print_string("Title:\t"); 
   print_string( m->name);

   set_cursor(MUSIC_Y+1,MUSIC_X);
   set_color(GREEN);
   print_string("Buf num:\t"); 
   print_int32( m->bufNum);

   set_cursor(MUSIC_Y+2,MUSIC_X);
   set_color(GREEN);
   print_string("Cur Time:\t"); 
   print_int32( m->bufNum / 86 /* 85.9375 */);

   set_cursor(MUSIC_Y+3,MUSIC_X);
   set_color(YELLOW);
   print_string("File Len:\t"); 
   print_int32(m->size);

   set_cursor(MUSIC_Y+4,MUSIC_X);
   set_color(GREEN);
   print_string("Song Time:\t"); 
   print_int32( m->size / 22000);


   set_cursor(MUSIC_Y+5,MUSIC_X);
   set_color(GREEN);
   print_string("Song Num:\t"); 
   print_int(m->songNum);


   set_cursor(MUSIC_Y+6,MUSIC_X );
   set_color(GREEN);
   int i;
   write_byte('[');
   for(i=0; i < BAR_LEN * (m->bufNum / 86 /* 85.9375 */) / m->size/22000; i++) {
      write_byte('#'); /* '█'); */
   }
   set_color(YELLOW);
   while(i++ < 50) {
      write_byte('_');
   }
   write_byte(']');
}


void printThreadsMain(music_t *music){
   while(1){
      printSys((system_t *) sys);
      printMusic(music);
      
      set_cursor(MUSIC_Y+7,0 );
      print_string("DEBUG: <");
      print_string(debug);
      print_string(">");
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
   struct ext2_inode inode;

   fileIndex = 0;
   
   while(1){
      //first read
      //set_cursor(49,0);
      //print_string("start");
      inodeNum = readRoot(&fileIndex, music->name, &music->size, &inode);
      /*set_cursor(50,0);
      print_string("start reading\tInodeNum: ");
      print_int32(inodeNum);*/
      readFile(inodeNum, music->bufNum, music->buf); 
      //set_cursor(51,0);
      //print_string("done reading");
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
            }else if( c == 'n') {
               fileIndex++;
	            music->songNum++;
	            break;
	         }
	      }  
	 	 
         //if queue needs to be filled
         if(music->readI/BUF_SIZE == music->playI/BUF_SIZE){
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

int main() {
   uint8_t sd_card_status;
   music_t music;

   /*while(1) {

      sd_card_status = sdInit(1);   //initialize the card with slow clock
      serial_init(); 
      print_int(sd_card_status);
      print_string(" sd card \n\r");

      uint8_t buf[512];
      memset( buf, 2, 512);
      sdReadData(10, 0, buf, 500);

      int i;
      for(i=0; i < 500; i++) {
	      print_int(buf[i]);
	      print_string(", ");
      }

   }*/
      
   serial_init();
   clear_screen();

   while(!(sd_card_status = sdInit(1))){   //1 for slow, o for fast */
      print_string("SD CARD IS BROKEN\r\n"); 
       uint64_t i; 
       for(i =0; i < 10000; i++) { 
    	 write_byte(0); 
       } 
    
    } 
   
   initMusic(&music); 
   /*VALIDATED (buzz)
   for(int i=0;i<512;i++){
      music.buf[i]=255*(i%2);
   }*/
   start_audio_pwm(); 
   sys = os_init_noMain();
   
   create_thread("playback", (uint16_t) &playbackMain, &music, 5);
   create_thread("reader", (uint16_t) &readMain, &music, 2500);
   create_thread("stats", (uint16_t) &printThreadsMain, &music, PRINT_THREAD_SIZE);
   create_thread("idle", (uint16_t) &idle, NULL, 5);

   os_start();
}
