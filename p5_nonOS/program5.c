#include <avr/io.h>
#include <avr/interrupt.h>
/* #include <stdint.h> */
#include <util/delay.h>
#include <stdlib.h>
/* #include "ext2.h" */
/* #include "SdReader.h" */
#include "ext2Luke.h"
#include "serial.h"


void start_audio_pwm();
void start_system_timer();
void play_file(int inode_num);
int main(void) {
   uint8_t sd_card_status;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 

   print_int( sd_card_status);



   
   int inode_num = 2;
   uint8_t buf[1024];
   inode_num = find_file("Evgeny_Grinko_-_05_-_Sunset.wav");
   print_int(inode_num);
   print_string("start of test\r\n");
   get_file_inode( inode_num, buf);
   print_string("mid test\r\n");
   /* print_dir( (struct ext2_inode *) buf); */

   
   
   start_system_timer();
   start_audio_pwm();
   sei();

   play_file(113);


   
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
   int size;
   get_file_inode( inode_num, buf);
   inode = (struct ext2_inode *) buf;

   size = inode->i_size;
   int i;
   for(i=0; i < 12; i++) {
      get_fs_block(inode->i_block[i],fsBlock);
      /* fwrite( fsBlock, sizeof(char), MIN(BLK_SIZE, size), stdout); */
      size -= BLK_SIZE;
      if(size <= 0) return;
   }

   /* singly indirect */
   get_fs_block(inode->i_block[i],indirect);
   for(i=0; i < 256; i++) {
      get_fs_block(indirect[i],fsBlock);
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
	 get_fs_block(indirect[i],fsBlock);
	 /* fwrite( fsBlock, sizeof(char), MIN(BLK_SIZE, size), stdout); */
	 size -= BLK_SIZE;
	 if(size <= 0) return;
      }
   }
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
   static uint8_t i =0;
   OCR2B = i++;

   if(i==0)
      print_string(".");
}
