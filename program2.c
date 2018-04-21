#include <string.h>

#include "globals.h"
#include "program2.h"
#include "blinkLED.h"
#include "printThreads.h"

#define REGSIZE 34  //17 ours? 32 total?

//void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size);


/* sys is now stored in heap space because of weird issue of sys not updating */
system_t * sys;

int main(int argc, char **argv){
   serial_init();
   os_init();

   create_thread("blink", (uint16_t) &blinkLEDMain, NULL, BLINK_LED_SIZE);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);

   printSys(sys);
   os_start();
}

void os_init(){
   sys = malloc(sizeof(system_t));
   sys->curThread=-1;
   sys->threadCount=0;
   sys->time=0;
}

void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size){
   regs_context_switch * regs;

   sys->threads[sys->threadCount].name=name;
   sys->threads[sys->threadCount].stackBase = (uint16_t)malloc(stack_size + REGSIZE);
   sys->threads[sys->threadCount].stackEnd =
      sys->threads[sys->threadCount].stackBase + stack_size + REGSIZE;

   sys->threads[sys->threadCount].stackPtr = (sys->threads[sys->threadCount].stackEnd
      - sizeof(regs_context_switch));
   
   //init stack
   regs = (regs_context_switch *) sys->threads[sys->threadCount].stackPtr;
   memset(regs, 0, sizeof(regs_context_switch));


   regs->pcl = (uint8_t) ((uint16_t)thread_start);
   regs->pch = (uint8_t) (((uint16_t)thread_start) >> 8);
   //zero upper byte of PC
   regs->eind = 0;              

   /* this is the stack */
   regs->r2 = (uint8_t) address; 
   regs->r3 = (uint8_t) (address >> 8); 

   regs->r4 = (uint8_t) ((uint16_t)args); 
   regs->r5 = (uint8_t) (((uint16_t)args) >> 8); 

   sys->threadCount++;
}

void os_start(){
   uint8_t next;
   uint16_t trash;
   //print_string("start\r\n");
   
   next = get_next_thread();
   sys->curThread = next;
   context_switch(&(sys->threads[next].stackPtr), &trash);
   //should never get here
}

uint8_t get_next_thread(){
   return (sys->curThread+1)%sys->threadCount;
}

__attribute__((naked)) void context_switch(uint16_t* new_sp, uint16_t* old_sp) {
   
   //push 
   asm volatile("push r2");
   asm volatile("push r3"); 
   asm volatile("push r4"); 
   asm volatile("push r5"); 
   asm volatile("push r6"); 
   asm volatile("push r7"); 
   asm volatile("push r8"); 
   asm volatile("push r9"); 
   asm volatile("push r10"); 
   asm volatile("push r11"); 
   asm volatile("push r12"); 
   asm volatile("push r13"); 
   asm volatile("push r14"); 
   asm volatile("push r15"); 
   asm volatile("push r16"); 
   asm volatile("push r17"); 
   asm volatile("push r28"); 
   asm volatile("push r29");
   
   

   //args are in r23,r22 (old) and r25,r24 (new)
   //stack pointer temp in r2,r3
   //Stack Pointer get->save->load->set

   
   //SAVE SP
   //get low SP
   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x5D");
   asm volatile("ld r2, Z ");
   //get high SP
   asm volatile("ldi r30, 0x5E");
   asm volatile("ld r3, Z");
   //move SP addr
   asm volatile("movw r30, r22");
   //save low SP
   asm volatile("st Z+, r2");
   //save hight SP
   asm volatile("st Z, r3");


   //LOAD SP
   //move SP addr
   asm volatile("movw r30, r24");
   //load low SP
   asm volatile("ld r2, Z+");
   //get high SP
   asm volatile("ld r3, Z");
   //set low SP
   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x5D");
   asm volatile("st Z, r2");
   //set high SP
   asm volatile("ldi r30, 0x5E");
   asm volatile("st Z, r3");


   //pop
   asm volatile("pop r29");
   asm volatile("pop r28"); 
   asm volatile("pop r17"); 
   asm volatile("pop r16"); 
   asm volatile("pop r15"); 
   asm volatile("pop r14"); 
   asm volatile("pop r13"); 
   asm volatile("pop r12"); 
   asm volatile("pop r11"); 
   asm volatile("pop r10"); 
   asm volatile("pop r9"); 
   asm volatile("pop r8"); 
   asm volatile("pop r7"); 
   asm volatile("pop r6"); 
   asm volatile("pop r5"); 
   asm volatile("pop r4"); 
   asm volatile("pop r3"); 
   asm volatile("pop r2");

   /*print_string("context");

   print_string("\n\r pc h l: ");
   print_string("\n\r ");
   print_hex32(*((uint8_t*)(sys->threads[0].stackEnd-1)));
   print_string("\t");
   print_hex32(*((uint8_t*)(sys->threads[0].stackEnd-2)));
   print_string("\t");
   print_hex32(*((uint8_t*)(sys->threads[0].stackEnd-3)));
   print_string("\n\r");
   
   print_string("\n\r addr check: ");
   print_hex32(*((uint8_t*)(sys->threads[0].stackEnd-4)));
   print_string("\n\r");
   */
   //return
   asm volatile("ret");
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
   
   sys->time++;
   next = get_next_thread();
   context_switch(&(sys->threads[next].stackPtr), &(sys->threads[sys->curThread].stackPtr));
   sys->curThread = next;

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

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave as the first statement in thread_start()
   //END SENG
   //ijump to given function address - (r2, r3) moved into Z vreg (r30, r31)
   //agrs moved from (r4, r5) to (r22, r23)
   asm volatile("movw r30, r2"); 
   asm volatile("movw r22, r4"); 
   asm volatile("ijmp");
}

