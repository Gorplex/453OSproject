#include "binkLED.h"


#define REGSIZE 34  //17 ours? 32 total?

system_t sys;

int main(int argc, char **argv){
   os_init();
   create_thread("Thread 0", &blinkLEDMian, NULL, BLINK_LED_SIZE);
   os_start();

}

void os_init(){
   sys.curThread=-1;
   sys.threadCount=0;
   sys.time=0;

}

void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size){
      thread_t *thr;

      thr = malloc(sizeof(thread_t));
      thr->name=name;
      //dont need?
      //thr->address=address;
      //thr->args->args;
      thr->stackTop = malloc(stack_size + REGSIZE);
      thr->stackBase = thr->stackTop + stack_size + REGSIZE;
      
      
      //init stack and stack pointer


      sys.thread[sys.threadCount] = thr;
      sys.threadCount++;
}

void os_start(){
   uint8_t next;
   uint16_t trash;

   next = get_next_thread();
   sys.curThread = next;
   context_switch(sys.threads[next]->stackPtr, &trash);
   //done?
}

uint8_t get_next_thread(){
   return (sys.curThread+1)%sys.threadCount
}

void context_switch(uint16_t *newSP, uint16_t *oldSP){
   //push
   asm volatile("push r29"); 
   asm volatile("push r28"); 
   asm volatile("push r17"); 
   asm volatile("push r16"); 
   asm volatile("push r15"); 
   asm volatile("push r14"); 
   asm volatile("push r13"); 
   asm volatile("push r12"); 
   asm volatile("push r11"); 
   asm volatile("push r10"); 
   asm volatile("push r9"); 
   asm volatile("push r8"); 
   asm volatile("push r7"); 
   asm volatile("push r6"); 
   asm volatile("push r5"); 
   asm volatile("push r4"); 
   asm volatile("push r3"); 
   asm volatile("push r2"); 

   //save old stack pointer
   //load new stack pointer 
   
   
   //pop
   asm volatile("pop r2"); 
   asm volatile("pop r3"); 
   asm volatile("pop r4"); 
   asm volatile("pop r5"); 
   asm volatile("pop r6"); 
   asm volatile("pop r7"); 
   asm volatile("pop r8"); 
   asm volatile("pop r9"); 
   asm volatile("pop r10"); 
   asm volatile("pop r11"); 
   asm volatile("pop r12"); 
   asm volatile("pop r13"); 
   asm volatile("pop r14"); 
   asm volatile("pop r15"); 
   asm volatile("pop r16"); 
   asm volatile("pop r17"); 
   asm volatile("pop r28"); 
   asm volatile("pop r29"); 

}

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   uint8_t next;
   
   //START SENG
   //At the beginning of this ISR, the registers r0, r1, and r18-31 have 
   //already been pushed to the stack

   //The following statement tells GCC that it can use registers r18-r31 
   //for this interrupt routine.  These registers (along with r0 and r1) 
   //will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        

   //Insert your code here
   //Call get_next_thread to get the thread id of the next thread to run
   //Call context switch here to switch to that next thread
   
   //At the end of this ISR, GCC generated code will pop r18-r31, r1, 
   //and r0 before exiting the ISR
   //END SENG
   
   next = get_next_thread();
   context_switch(sys.threads[next]->stackPtr, sys.threads[sys.curThread]->stackPtr);
   sys.curThread = next;

}

//START SENG
//Call this to start the system timer interrupt
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00) | _BV(CS02);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds
}

__attribute__((naked)) void context_switch(uint16_t* new_sp, uint16_t* old_sp) {
}

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave as the first statement in thread_start()
}
//END SENG



