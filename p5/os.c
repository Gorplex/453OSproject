/* Written: Luke Thompson and John Thomsen */

#include "os.h"

/* sys is now stored in heap space because of weird issue of sys not updating */
system_t * sys;

system_t * os_init(){
   cli();
   start_system_timer();
   cli();
   sys = malloc(sizeof(system_t));
   sys->curThread=0;    // was -1,0 now 0,1
   sys->threadCount=1;  //main is now a thread
   sys->time=0;
   sys->mtime=0;
   sys->threads[0].name="main";
   sys->threads[0].pc=0;
   sys->threads[0].stackBase=0;
   sys->threads[0].stackEnd=0x21FF;
   sys->threads[0].stackPtr=0;
   sys->threads[0].thread_status=THREAD_RUNNING;
   sys->threads[0].cur_count=0;
   sys->threads[0].sched_count=0;
   sys->threads[0].wakeup_time=0;
   sys->cur_count=0;
   sys->sched_count=0;
   return sys;
}

system_t * os_init_noMain(){
   cli();
   start_system_timer();
   cli();
   sys = malloc(sizeof(system_t));
   sys->curThread=NOT_THREAD;
   sys->threadCount=0;  
   sys->time=0;
   sys->mtime=0;
   sys->cur_count=0;
   sys->sched_count=0;
   return sys;
}

void create_thread(char* name, uint16_t address, void* args, uint16_t stack_size){
   regs_context_switch * regs;
   //setup sys thread
   sys->threads[sys->threadCount].name=name;
   sys->threads[sys->threadCount].pc=address;
   sys->threads[sys->threadCount].stackBase = (uint16_t)malloc(stack_size + REGSIZE);
   sys->threads[sys->threadCount].stackEnd =
      sys->threads[sys->threadCount].stackBase + stack_size + REGSIZE;
   sys->threads[sys->threadCount].stackPtr = (sys->threads[sys->threadCount].stackEnd
      - sizeof(regs_context_switch));
   sys->threads[sys->threadCount].thread_status=THREAD_READY;
   sys->threads[sys->threadCount].cur_count=0;
   sys->threads[sys->threadCount].sched_count=0;
   sys->threads[sys->threadCount].wakeup_time=0;
   
   //setup stack
   regs = (regs_context_switch *) sys->threads[sys->threadCount].stackPtr;
   memset(regs, 0, sizeof(regs_context_switch));

   regs->pcl = (uint8_t) ((uint16_t)thread_start);
   regs->pch = (uint8_t) (((uint16_t)thread_start) >> 8);
   //zero upper byte of PC
   regs->eind = 0;              

   regs->r2 = (uint8_t) address; 
   regs->r3 = (uint8_t) (address >> 8); 
   regs->r4 = (uint8_t) (uint16_t) args; 
   regs->r5 = (uint8_t) (((uint16_t)args) >> 8); 

   sys->threadCount++;
}

void os_start(){
   uint16_t trash;
   //if cur thread is 0 main needs to be saved if -1 main context is discarded
   if(sys->curThread==NOT_THREAD){
      sys->curThread = get_next_thread();
      sys->threads[sys->curThread].thread_status = THREAD_RUNNING;
      sys->threads[sys->curThread].cur_count++;
      sys->cur_count++;
      //interrupts enabled in each thread start 
      context_switch(&(sys->threads[sys->curThread].stackPtr), &trash);
   }else{
      yield();
   }
}

//project 3 func
void check_sleeping_threads(){
   int i;
   for(i=0;i<sys->threadCount;i++){
      if(sys->threads[i].thread_status==THREAD_SLEEPING && 
         sys->threads[i].wakeup_time <= sys->mtime){
         sys->threads[i].wakeup_time = 0;
         sys->threads[i].thread_status = THREAD_READY;
      }
   }
}

//if no valid thread to swap to returns same thread
//because it is called while interrupts are disabled
/*
uint8_t get_next_thread(){
   uint8_t next;
   check_sleeping_threads();
   next = (sys->curThread+1)%sys->threadCount;
   while(sys->threads[next].thread_status != THREAD_READY
      && sys->threads[next].thread_status != THREAD_RUNNING){
      next = (next+1)%sys->threadCount;
      if(next == sys->curThread){
         break;
      }
   }
   return next;
}*/
//Project 5 PRIORITY

uint8_t get_next_thread(){
   uint8_t next;
   //check_sleeping_threads();   MOVED TO ISR FOR SPEED
   next = 0;   //next always starts at thread 0, priority queue
   while(sys->threads[next].thread_status != THREAD_READY
      && sys->threads[next].thread_status != THREAD_RUNNING){
      next = (next+1)%sys->threadCount;
      if(next == sys->curThread){
         break;
      }
   }
   return next;
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

   //return
   asm volatile("ret");
}

//START SENG
//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
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
   
   sys->mtime += MS_PER_TICK;
   check_sleeping_threads();
   thread_swap(get_next_thread());
}

//next two functions for p3
void update_sched_count(){
   int i;
   sys->sched_count = sys->cur_count;   
   sys->cur_count = 0;   
   for(i=0;i<sys->threadCount;i++){
      sys->threads[i].sched_count = sys->threads[i].cur_count; 
      sys->threads[i].cur_count = 0; 
   }
}

ISR(TIMER1_COMPA_vect) {
   //This interrupt routine is run once a second
   //The 2 interrupt routines will not interrupt each other
   sys->time++;
   update_sched_count();
}

//START SENG
//PROJECT 2-3 CODE (QUANTUM 10)
/*void start_system_timer() {
   //start timer 0 for OS system interrupt
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds

   //start timer 1 to generate interrupt every 1 second
   OCR1A = 15625;
   TIMSK1 |= _BV(OCIE1A);  //interrupt on compare
   TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024
}*/

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave as the first statement in thread_start()
   //END SENG
   //ijump to given function address - (r2, r3) moved into Z vreg (r30, r31)
   //agrs moved from (r4, r5) to (r22, r23)
   asm volatile("movw r30, r2"); 
   asm volatile("movw r24, r4"); 
   asm volatile("ijmp");

   cli();//may not be nessary alows thread to end by yield() rather than by ISR
   sys->threads[sys->curThread].thread_status = THREAD_ENDED;
   yield();
}

//project 3
//currently passes (does nothing with 0)
//anything less than MS_PER_TICK is equivlent to yield() (with some insignificant overhead)
void thread_sleep(uint16_t ticks){
   TID_T next;
   
   if(ticks){
      cli();
      sys->threads[sys->curThread].wakeup_time = sys->mtime + ticks*MS_PER_TICK;
      sys->threads[sys->curThread].thread_status=THREAD_SLEEPING;
      
      next = get_next_thread();
      //IF ALL THREADS ARE UNAVALABLE 
      //ENABLE INTERRUPTS (to keep time) THEN STALL
      //ISR THREAD SWAP WILL COME BACK HERE UNLESS ANOTHER THREAD IS AVALABLE 
      //THEN IT WILL NOT RETURN UNTILL THIS IS AVALABLE (able to leave loop)
      //(if new thread sleeps it will loop int that thread instead)
      if(sys->curThread == next
         && sys->threads[next].thread_status != THREAD_READY
         && sys->threads[next].thread_status != THREAD_RUNNING){
         sei();
         while(sys->threads[next].thread_status != THREAD_READY
            && sys->threads[next].thread_status != THREAD_RUNNING){
            //OS STALLS HERE WHEN NO THREADS WANT TO RUN
            //INTERRUPTS ENABLED OS TIME KEEPS TICKING
            //LOW POWER MODE GOES HERE (may need to revert to normal on ISR)
            //avr-gcc was optomising out my while loop!!
            asm volatile("nop");
         }
         cli(); 
      }  
      thread_swap(next);
      sei();
   }
}

void yield(){
   cli(); 
   thread_swap(get_next_thread());
   sei();
}

uint16_t get_thread_id(){
   return sys->curThread;   
}

//should only be called when interrupts are disabled
//call yield when interrupts are enabled
void thread_swap(TID_T next){
   TID_T last;
   
   sys->threads[next].cur_count++;
   sys->cur_count++;
   if(sys->curThread != next){
      last = sys->curThread;
      sys->curThread = next;
      if(sys->threads[last].thread_status == THREAD_RUNNING){
         sys->threads[last].thread_status = THREAD_READY;
      }
      sys->threads[sys->curThread].thread_status = THREAD_RUNNING;
      context_switch(&(sys->threads[sys->curThread].stackPtr), &(sys->threads[last].stackPtr));
   }
}
void malloc_thread_stack(TID_T tid, uint16_t stack_size){
   sys->threads[tid].stackBase = (uint16_t)malloc(stack_size + REGSIZE);
}

void create_thread_live(char* name, uint16_t address, void* args, uint16_t stack_size){
   regs_context_switch * regs;
   
   cli();
   //setup sys thread
   sys->threads[sys->threadCount].name=name;
   sys->threads[sys->threadCount].pc=address;
   //left out for live
   //sys->threads[sys->threadCount].stackBase = (uint16_t)malloc(stack_size + REGSIZE);
   sys->threads[sys->threadCount].stackEnd =
      sys->threads[sys->threadCount].stackBase + stack_size + REGSIZE;
   sys->threads[sys->threadCount].stackPtr = (sys->threads[sys->threadCount].stackEnd
      - sizeof(regs_context_switch));
   sys->threads[sys->threadCount].thread_status=THREAD_READY;
   sys->threads[sys->threadCount].cur_count=0;
   sys->threads[sys->threadCount].sched_count=0;
   sys->threads[sys->threadCount].wakeup_time=0;
   
   //setup stack
   regs = (regs_context_switch *) sys->threads[sys->threadCount].stackPtr;
   memset(regs, 0, sizeof(regs_context_switch));

   regs->pcl = (uint8_t) ((uint16_t)thread_start);
   regs->pch = (uint8_t) (((uint16_t)thread_start) >> 8);
   //zero upper byte of PC
   regs->eind = 0;              

   regs->r2 = (uint8_t) address; 
   regs->r3 = (uint8_t) (address >> 8); 
   regs->r4 = (uint8_t) (uint16_t) args; 
   regs->r5 = (uint8_t) (((uint16_t)args) >> 8); 

   sys->threadCount++;

   sei();
}

//PROJECT 5 CODE (from sengs os_util.c)
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  /* IRQ on compare.  */
   TCCR0A |= _BV(WGM01); //clear timer on compare match

   //22KHz settings
   TCCR0B |= _BV(CS01); //prescalar /8
   OCR0A = 90; //generate interrupt every 45 microseconds

   //start timer 1 to generate interrupt every 1 second
   OCR1A = 15625;
   TIMSK1 |= _BV(OCIE1A);  /* IRQ on compare.  */
   TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024
}

void start_audio_pwm() {
   //run timer 2 in fast pwm mode
   TCCR2A |= _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
   TCCR2B |= _BV(CS20);

   DDRH |= _BV(PH6); //make OC2B an output
}
