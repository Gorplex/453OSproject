/* Written: Luke Thompson and John Thomsen and John Seng*/
#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>

#define REGSIZE 45  //41 used with empty function

#define THREAD_RUNNING  0     //thread is currently running
#define THREAD_READY     1    //thread is ready to be run
#define THREAD_SLEEPING  2    //set from call to thread_sleep()
#define THREAD_WAITING   3    //waiting on mutex or semaphore

#define MS_PER_TICK 10        //10 ms between each thread swap

//This structure defines the register order pushed to the stack on a
//system context switch.
typedef struct regs_context_switch {
   uint8_t padding; //stack pointer is pointing to 1 byte below the top of the stack

   //Registers that will be managed by the context switch function
   uint8_t r29;
   uint8_t r28;
   uint8_t r17;
   uint8_t r16;
   uint8_t r15;
   uint8_t r14;
   uint8_t r13;
   uint8_t r12;
   uint8_t r11;
   uint8_t r10;
   uint8_t r9;
   uint8_t r8;
   uint8_t r7;
   uint8_t r6;
   uint8_t r5;
   uint8_t r4;
   uint8_t r3;
   uint8_t r2;
   uint8_t eind;
   uint8_t pch;
   uint8_t pcl;
} regs_context_switch;

//This structure defines how registers are pushed to the stack when
//the system 10ms interrupt occurs.  This struct is never directly
//used, but instead be sure to account for the size of this struct
//when allocating initial stack space
typedef struct regs_interrupt {
   uint8_t padding; //stack pointer is pointing to 1 byte below the top of the stack

   //Registers that are pushed to the stack during an interrupt service routine
   uint8_t r31;
   uint8_t r30;
   uint8_t r27;
   uint8_t r26;
   uint8_t r25;
   uint8_t r24;
   uint8_t r23;
   uint8_t r22;
   uint8_t r21;
   uint8_t r20;
   uint8_t r19;
   uint8_t r18;
   uint8_t rampz; //rampz
   uint8_t sreg; //status register
   uint8_t r0;
   uint8_t r1;
   uint8_t eind;
   uint8_t pch;
   uint8_t pcl;
} regs_interrupt;



typedef struct thread_t {
   char * name;
   uint16_t pc;
   uint16_t stackPtr;
   uint16_t stackBase;
   uint16_t stackEnd;
   uint16_t thread_status;
   uint16_t sched_count;
   uint16_t sleep_timer;
} thread_t;

typedef struct system_t {
   thread_t threads[8];
   int8_t curThread;
   uint8_t threadCount;
   uint32_t time;
} system_t;

system_t * os_init(void);
void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size);
void os_start(void);
uint8_t get_next_thread(void);
void thread_start(void);
__attribute__((naked)) void context_switch(uint16_t* new_sp, uint16_t* old_sp);
void start_system_timer();

//project 3
void thread_sleep(uint16_t ticks);
void yield();
uint16_t get_thread_id();



#endif
