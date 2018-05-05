/* Written: Luke Thompson and John Thomsen */

#include "os.h"
#include "printThreads.h"

#define BUF_TS 50
#define PROD_TS 5
#define CONS_TS 5
#define BLINK_TS 0

#define BUF_SIZE 50        //circular queue
#define BASE_DELAY 1000    //ms initial delay for producer and consumer
#define DELAY_INREMENT 50  //ms each keypress

typedef struct buffer_t {
   uint16_t prod_delay;
   uint16_t cons_delay;
   uint16_t start;
   uint16_t size;
   uint16_t buf[BUF_SIZE];
} buffer_t;

void display_bounded_buffer(buffer_t *buf){
   //serial_init();
   while(1){

   }
}
void producer(buffer_t *buf){
   while(1){
      if(buf->size < BUF_SIZE){
         //make a number at [start+size]
         buf->size++;
         //thread_sleep(buf->prod_delay/MS_PER_TICK)
      }else{
         //wait for consumer
      }
   }
}

void consumer(buffer_t *buf){
   while(1){
      if(buf->size){
         //remove a number at [start]
         buf->start=(buf->start+1)%BUF_SIZE;
         buf->size--;
         //thread_sleep(buf->cons_delay/MS_PER_TICK)
      }else{
         //wait for producer
      }
   }
}

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
void blink(buffer_t *buf){
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
   buf->prod_delay=BASE_DELAY;
   buf->cons_delay=BASE_DELAY;
   buf->start=0;
   buf->size=0;

   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("producer", (uint16_t) &producer, buf, PROD_TS);
   create_thread("consumer", (uint16_t) &consumer, buf, CONS_TS);
   create_thread("blink", (uint16_t) &blink, buf, BLINK_TS);

   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

