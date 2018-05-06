/* Written: Luke Thompson and John Thomsen */

#include "os.h"
#include "printThreads.h"
#include "serial.h"
#include "synchro.h"


#define BUF_TS 50
#define PROD_TS 5
#define CONS_TS 5
#define BLINK_TS 0

#define BUF_SIZE 50        //circular queue
#define BASE_DELAY 1000    //ms initial delay for producer and consumer
#define DELAY_INREMENT 50  //ms each keypress

#define RAND_RANGE 1000    //posible numbers between 0 and RAND_RANGE-1

typedef struct buffer_t {
   uint16_t prod_delay;
   uint16_t cons_delay;
   uint16_t start;
   uint16_t size;
   uint16_t buf[BUF_SIZE];
} buffer_t;



struct mutex_t * screem;		/* screen mutex */


void display_bounded_buffer(buffer_t *buf){
   serial_init();
   int i=0;
   while(1){
      mutex_lock(screem);
      set_color(RED);
      set_cursor(4,74);
      print_string("________");
      for(i=0; i < BUF_SIZE; i++) {
	 set_cursor(5+i,74);
	 print_string("|");
	 print_int(buf->buf[i]);
	 print_string("\t|");
      }
      set_cursor(5+BUF_SIZE,74);
      print_string("|______|");
      
      mutex_unlock(screem);
   }
}
void producer(buffer_t *buf){
   while(1){
      if(buf->size < BUF_SIZE){
         //make a number at [start+size]
         buf->buf[buf->start+buf->size] = rand()%RAND_RANGE;
         buf->size++;
         thread_sleep(buf->prod_delay/MS_PER_TICK);
      }else{
         //wait for consumer
         yield();
      }
   }
}

void consumer(buffer_t *buf){
   while(1){
      if(buf->size){
         //remove a number at [start]
         buf->start=(buf->start+1)%BUF_SIZE;
         buf->size--;
         thread_sleep(buf->cons_delay/MS_PER_TICK);
      }else{
         //wait for producer
         yield();
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
      yield();
   }
}


int main(int argc, char **argv){
   system_t * sys;
   buffer_t * buf;
   screem = malloc(100);//sizeof(mutex_t)); /* HELP: TODO: */
   mutex_init(screem);
   sys = os_init();
   buf->prod_delay=BASE_DELAY;
   buf->cons_delay=BASE_DELAY;
   buf->start=0;
   buf->size=0;

   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   create_thread("producer", (uint16_t) &producer, buf, PROD_TS);
   create_thread("consumer", (uint16_t) &consumer, buf, CONS_TS);
   create_thread("blink", (uint16_t) &blink, buf, BLINK_TS);
   create_thread("disp buf", (uint16_t) &display_bounded_buffer, buf, PRINT_THREAD_SIZE);
   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

