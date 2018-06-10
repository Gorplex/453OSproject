#include <avr/io.h>
#include <avr/interrupt.h>
/* #include <stdint.h> */
#include <util/delay.h>
#include <stdlib.h>
/* #include "ext2.h" */
/* #include "SdReader.h" */
#include "ext2Luke.h"
#include "serial.h"

volatile uint16_t index = 0;
volatile uint8_t musicBuf[1024];
uint8_t songIndex;

void start_audio_pwm();
void start_system_timer();
uint8_t updateUI(uint32_t played, uint32_t total);
void play_file(int inode_num);
void play_fs_block(uint32_t block);
int main(void) {
   uint8_t sd_card_status;
   int inode_num = 2;
   uint8_t buf[1024];
   uint32_t inodes[32];

   sd_card_status = sdInit(1);   //initialize the card with slow clock
   serial_init(); 

   if(!sd_card_status) {
      print_string("sd init failed\r\n");
      while(1);
   }
   
   memset(musicBuf, 0, 1024);
   memset(inodes, 0, 32 * sizeof(uint32_t));

   clear_screen();

   get_file_inode( 2, buf);
   print_dir( (struct ext2_inode *) buf, inodes);

  

   /* int i; */
   /* for(i=0; i < 32; i++) { */
   /*    print_int32(inodes[i]); */
   /*    print_string("\r\n"); */
   /* } */
   
   
   start_system_timer();
   start_audio_pwm();
   sei();

   while(1) {
      play_file(inodes[songIndex]);
      set_cursor(songIndex,0);
      print_string("      ");

      songIndex++;
      if(inodes[songIndex] == 0) {
	 songIndex=0;
      }

   }

   memset(musicBuf, 0, 1024);
   print_string("end of test\r\n");
   while(1);
   return 0;
}




void play_file(int inode_num) {
   struct ext2_inode * inode;
   uint8_t buf[BLK_SIZE];
   uint8_t fsBlock[BLK_SIZE];
   uint32_t indirect[BLK_SIZE/4];
   uint32_t doubly_indirect[BLK_SIZE/4];
   int32_t size, totalSize;
   get_file_inode( inode_num, buf);
   inode = (struct ext2_inode *) buf;

   totalSize = size = inode->i_size;
   int i;
   index=0;
   for(i=0; i < 12; i++) {
      play_fs_block(inode->i_block[i]);
      if(updateUI(totalSize - size, size))
	 return;
      while(index<1024);
      index=0;
      size -= BLK_SIZE;
      if(size <= 0) return;
   }

   /* singly indirect */
   get_fs_block(inode->i_block[i],indirect);
   for(i=0; i < 256; i++) {
      play_fs_block(indirect[i]);
      if(updateUI(totalSize - size, totalSize))
	 return;
      while(index<1024);
      index=0;

      /* print_string("sing\r\n"); */
      /* fwrite( fsBlock, sizeof(char), MIN(size,BLK_SIZE), stdout); */
      size -= BLK_SIZE;
      if(size <= 0) return;
   }

   /* doubly indirect */
   get_fs_block(inode->i_block[13],doubly_indirect);
   int j;
   for(j=0; j < 256; j++) {

      get_fs_block(doubly_indirect[j],indirect);
      for(i=0; i < 256; i++) {
	 play_fs_block(indirect[i]);
	 /* print_string("doub\r\n"); */
	 if(updateUI(totalSize - size, totalSize))
	    return;
	 while(index<1024);
	 index=0;
	 /* fwrite( fsBlock, sizeof(char), MIN(BLK_SIZE, size), stdout); */
	 size -= BLK_SIZE;
	 if(size <= 0) return;
      }
   }
}


void play_fs_block(uint32_t block) {
   /* read_data */
   sdReadData( 2 * block     , 0, musicBuf+512,     512);

   while(index <= 512);

   /* read_data */
   sdReadData( 2 * block + 1 , 0, musicBuf, 512);

   /* maybe do stuff here? */

}

uint8_t updateUI(uint32_t played, uint32_t total) {
   uint8_t c;
   set_cursor(songIndex,0);
   print_string("  >>>>");
   set_cursor(24,0);
   print_string("Time (sec): ");
   print_int32( played /(21*1024));
   print_string(" / ");
   print_int32( total /(21*1024));
   print_string("\t\t");
   /* print_cmd("K"); */
   
   set_cursor(24,22);
   write_byte('[');
   int i;
   for(i=0; i <= 50 * played / total; i++) {
      write_byte('#');
   }
   while(i++ < 50) {
      write_byte('_');
   }
   print_string("] % ");   
   print_int( 100*played/total);

   
   c = read_byte();
   if( c == 'n') {
      /* songIndex++; */
      return 1;
   } else if( c == 'p') {
      if(songIndex != 0) {
	 set_cursor(songIndex,0);
	 print_string("      ");
	 songIndex-=2;
      }
      return -1;
   }

   set_cursor(30,0);
   while(index<1000) print_string(".");
   return 0;
}


//PROJECT 5 CODE (from sengs os_util.c)
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  /* IRQ on compare.  */
   TCCR0A |= _BV(WGM01); //clear timer on compare match

   //22KHz settings
   TCCR0B |= _BV(CS01); //prescalar /8
   OCR0A = 90; //generate interrupt every 45 microseconds

   //start timer 1 to generate interrupt every 1 second
   /* OCR1A = 15625; */
   /* TIMSK1 |= _BV(OCIE1A);  /\* IRQ on compare.  *\/ */
   /* TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024 */
}

void start_audio_pwm() {
   //run timer 2 in fast pwm mode
   TCCR2A |= _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
   TCCR2B |= _BV(CS20);

   DDRH |= _BV(PH6); //make OC2B an output
}


ISR(TIMER0_COMPA_vect) {
   /* static uint8_t i =0; */
   OCR2B = musicBuf[index++ & 0x3FF];

   /* if(i==0) */
   /*    print_string("."); */
}
