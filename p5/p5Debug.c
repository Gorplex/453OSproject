#include <avr/io.h>
#include <avr/interrupt.h>
/* #include <stdint.h> */
#include <util/delay.h>
#include <stdlib.h>
/* #include "ext2.h" */
#include "SdReader.h"

#include "serial.h"

int main(void) {
   uint8_t sd_card_status;

   sd_card_status = sdInit(1);   //initialize the card with slow clock

   serial_init(); 

   print_int( sd_card_status);
   
   print_string("end of test\r\n");
   
   return 0;
}
