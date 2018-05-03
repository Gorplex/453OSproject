/* Written: Luke Thompson and John Thomsen */

#include "os.h"
#include "printThreads.h"


#define BUF_SIZE 50

#define BUF_TS 50
#define PROD_TS 50
#define CONS_TS 50
#define BLINK_TS 5

typedef struct buffer_t {
   uint16_t size;
   uint16_t buf[BUF_SIZE];
} buffer_t;


//BLINK LED CODE
void LED_init(){
   //set bit 7 to output
   asm volatile("ldi r31, 0");
   asm volatile("ldi r30, 0x24");
   
   asm volatile("ld  r18, Z");
   asm volatile("ori r18, 0x80");
   asm volatile("st    Z, r18");
}
void LED_on(){
   asm volatile("ldi r31, 0");
   asm volatile("ldi r30, 0x25");
   asm volatile("ld  r18, Z");
   asm volatile("ori r18, 0x80");
   asm volatile("st    Z, r18");
}
void LED_off(){
   asm volatile("ldi  r31, 0");
   asm volatile("ldi  r30, 0x25");
   asm volatile("ld   r18, Z");
   asm volatile("andi r18, 0x7f");
   asm volatile("st     Z, r18");
}
void blink(uint16_t * buffer){
   buffer_t *buf;
   buf=(buffer_t *)buffer;
   LED_init();
   while(1){
      if(buf->size < BUF_SIZE){
         LED_on();
      }else{
         LED_off();
      }
   }
}


int main(int argc, char **argv){
   system_t * sys;
   buffer_t * buf;

   sys = os_init();
   buf->size=0;

   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   create_thread("blink", (uint16_t) &blink, buf, BLINK_TS);
   
   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

