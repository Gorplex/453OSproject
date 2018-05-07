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
#define PROD_DELAY 1000    //ms initial delay for producer
#define CONS_DELAY 2000    //ms initial delay for consumer
#define DELAY_INREMENT 50  //ms each keypress

#define RAND_RANGE 1000    //posible numbers between 0 and RAND_RANGE-1

typedef struct buffer_t {
   uint16_t prod_delay;
   uint16_t cons_delay;
   uint16_t start;
   uint16_t size;
   uint16_t buf[BUF_SIZE];
   semaphore_t * notEmpty;
   semaphore_t * notFull;
} buffer_t;

//defined in print threads
extern mutex_t * screem;		/* screen mutex */

void display_bounded_buffer(buffer_t *buf){
   serial_init();
   int i=0;
   while(1){
      mutex_lock(screem);
      set_color(GREEN);
      set_cursor(3,72);
      print_string("SIZE: "); 
      print_int(buf->size);
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
   serial_init();
   while(1){
      //wait for consumer
      sem_wait(buf->notFull);
      //make a number at [start+size]
      buf->buf[buf->start+buf->size] = rand()%RAND_RANGE;
      buf->size++;
      
      /*mutex_lock(screem);
      set_cursor(4+buf->start,60);
      print_string("     ");
      set_cursor(5+buf->start,60);
      print_string("ADDED ");
      print_int_padded(buf->buf[buf->start+buf->size]);
      print_string(" ---");
      mutex_unlock(screem);
      */
      sem_signal(buf->notEmpty);   
      thread_sleep(buf->prod_delay/MS_PER_TICK);
   }
}

void consumer(buffer_t *buf){
   serial_init();
   while(1){
      //wait for producer
      sem_wait(buf->notEmpty);
      //remove a number at [start]
      buf->start=(buf->start+1)%BUF_SIZE;
      buf->size--;
      
      sem_signal(buf->notFull); 
      thread_sleep(buf->cons_delay/MS_PER_TICK);
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
   
   sys = os_init();
   
   screem = malloc(sizeof(mutex_t)); 
   mutex_init(screem);
   
   //buffer starts empty
   buf=calloc(1,sizeof(buffer_t));
   buf->prod_delay=PROD_DELAY;
   buf->cons_delay=CONS_DELAY;
   buf->start=0;
   buf->size=0;
   buf->notEmpty=malloc(sizeof(semaphore_t));
   sem_init(buf->notEmpty, 0);
   buf->notFull=malloc(sizeof(semaphore_t));
   sem_init(buf->notFull, BUF_SIZE);

   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   create_thread("display bufffer", (uint16_t) &display_bounded_buffer, buf, PRINT_THREAD_SIZE);
   create_thread("producer", (uint16_t) &producer, buf, PROD_TS);
   create_thread("consumer", (uint16_t) &consumer, buf, CONS_TS);
   create_thread("blink", (uint16_t) &blink, buf, BLINK_TS);
   
   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

