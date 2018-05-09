/* Written: Luke Thompson and John Thomsen */

#include <stdlib.h>
#include "synchro.h"
#include "printThreads.h"

#define INDEX uint8_t
#define ARRAY_SIZE 128
#define MAX_VAL 255
#define NUM_THREADS 4   //number of sorting threads

#define SORT_TS 500
#define DISP_TS 200

#define SHORT_DELAY 100
#define MED_DELAY 200
#define LONG_DELAY 500

uint8_t * copy;         //storage array
extern struct mutex_t * screem;

typedef struct signals_t {
   mutex_t *initThreads;
   uint8_t numCreated;
   uint8_t takenID;
   semaphore_t *done;
} signals_t;

signals_t * signals;
extern system_t *sys;

void printThreadsMain(uint16_t *sys){
   while(1){
      mutex_lock(screem);
      printSys((system_t *) sys);
      mutex_unlock(screem);
   }
}

void init_signals(signals_t * s){
   s->initThreads = malloc(sizeof(mutex_t));
   mutex_init(s->initThreads);
   s->numCreated = 1;
   s->takenID = 0;
   s->done = malloc(sizeof(semaphore_t));
   sem_init(s->done, 1-NUM_THREADS);
}

void init_array(uint8_t *array){
   uint16_t i;
   for(i=0;i<ARRAY_SIZE;i++){
      array[i] = rand()%MAX_VAL;
   }
}

void merge(uint8_t *array, INDEX start, INDEX mid, INDEX end, uint8_t * sorted){
   INDEX i,j;
   j = mid;
   for(i=start;i<end;i++){
      if(start < mid && (j >= end || array[start] <= array[j])){
         sorted[i] = array[start];
         start++;
      }else{
         sorted[i] = array[j];
         j++;
      }
   }
}

void sort(uint8_t *array, INDEX start, INDEX end, uint8_t *sorted){
   INDEX mid;
   //thread_sleep(50);
   if(end-start > 1){
      mid = (end-start)/2+start;
      sort(sorted, start, mid, array);
      //thread_sleep(SHORT_DELAY);
      sort(sorted, mid, end, array);
      //thread_sleep(SHORT_DELAY);
      merge(array, start, mid, end, sorted);
      //thread_sleep(SHORT_DELAY);
   }
}

void staller(){
   //cli();
   while(1){
      //set_cursor(40,60);
      //print_string("THIS IS PRINTING");
   }
}

void st_sort(uint8_t * array){
   INDEX i;
   
   while(1){ 
      thread_sleep(MED_DELAY);
      for(i=0;i<ARRAY_SIZE;i++){
         copy[i]=array[i];
      }
      thread_sleep(MED_DELAY);
      sort(copy, 0, ARRAY_SIZE, array);
      thread_sleep(LONG_DELAY);
      init_array(array);
      mutex_lock(screem);
      set_cursor(60,40);
      print_string("random");
      mutex_unlock(screem);
   }
}

void mt_sort(uint8_t *array){
   TID_T myID;
   uint16_t start;
   uint16_t end;
   uint16_t i;
   
   printm(GREEN, 69+signals->numCreated, 48,"Run");
   mutex_lock(signals->initThreads);
   signals->takenID++;
   myID = signals->takenID;
   start = ARRAY_SIZE*(myID-1)/NUM_THREADS; 
   end = ARRAY_SIZE*(myID)/NUM_THREADS; 
   if(myID == 1){
      printm(GREEN, 70, 40,"T1:");
      printm(GREEN, 71, 40,"T2:");
      printm(GREEN, 72, 40,"T3:");
      printm(GREEN, 73, 40,"T4:");  
      printm(RED, 69, 20,"--MASTER LOG--");  
   }
   while(signals->numCreated < NUM_THREADS){
      
      thread_sleep(MED_DELAY);
      signals->numCreated++;
      sys->threadCount++;
      //create_thread_live("sort spawned", (uint16_t) &mt_sort, array, SORT_TS);
      printm(GREEN, 69+signals->numCreated, 44,"Cre");

      //ALL CRAZY TESTS
      //create_thread_live("staller spawned", (uint16_t) &staller, NULL, 100);
      //cli();
      //context_switch(&(sys->threads[3].stackPtr), &(sys->threads[3].stackPtr));
      
      //sys->threads[3].wakeup_time = 5000;
      //sys->threads[3].thread_status = THREAD_SLEEPING;
      //printSys(sys); 

      /*set_cursor(56,40);
      set_cursor(59,40);
      print_string("IM HERE 1");
      //yield(); */
      /*
      i=0;
      while(1){
         mutex_lock(screem);
         set_cursor(60,40);
         print_string("IM HERE");
         set_cursor(61,40);
         print_int(i++);
         yield();
      }*/
   }
   while(1){
      if(myID==1){
         printm(GREEN, 70, 20,"Done Creating");
         thread_sleep(MED_DELAY);
         for(i=0;i<ARRAY_SIZE;i++){
            copy[i]=array[i];
         }
         printm(GREEN, 71, 20,"Sorted -> Copy");
         thread_sleep(MED_DELAY);
      }
      mutex_unlock(signals->initThreads);
      //ALL START
      printm(GREEN,69+myID, 52,"Start");
      
      sort(copy, start, end, array); 
      
      printm(GREEN,69+myID, 58,"Done");
      
      if(myID==1){
         mutex_lock(signals->initThreads); //for restarting
         printm(GREEN, 72, 20,"T1 DONE TAKEN MUTEX");
         thread_sleep(1000);
      }
      
      //wait for all threads here then all continue to get stuck on mutex
      sem_signal(signals->done);
      sem_wait(signals->done);
      sem_signal(signals->done);
      printm(GREEN,76+myID, 65,"Synced");
      
      //if not ID 1 get stoped else ID 1 finishes and inits 
      if(myID != 1){
         mutex_lock(signals->initThreads); //for restarting
      }else{//if(myID==1){
         merge(array, 0, ARRAY_SIZE/4, ARRAY_SIZE/2, copy);
         merge(array, ARRAY_SIZE/2, ARRAY_SIZE*3/4, ARRAY_SIZE, copy);
         merge(copy, 0, ARRAY_SIZE/2, ARRAY_SIZE, array);
         thread_sleep(1000);
         
         //restart
         init_array(array);
         
         //reset sem after you make sure all made it through
         sem_wait(signals->done);
         sem_init(signals->done, 1-NUM_THREADS);
      }
   }
}

void printArrayCol(uint16_t row,uint16_t col, uint8_t * array, INDEX index){
   INDEX i;
   for(i=0;i<ARRAY_SIZE/4;i++){
      set_cursor(row+i,col);
      print_int_spaces(array[i+index]);
   }
}

void displayMain(uint8_t *array){
   while(1){
      mutex_lock(screem);
      set_color(BR_RED);
      set_cursor(3,55);
      print_string("sort");
      set_cursor(3,70);
      print_string("sort");
      
      set_color(RED);
      set_cursor(3,60);
      print_string("copy");
      set_cursor(3,75);
      print_string("copy");

      set_color(BR_CYAN);
      printArrayCol(4,55,array,0);
      set_color(BR_MAGENTA);
      printArrayCol(5+ARRAY_SIZE/4,55,array,ARRAY_SIZE/4);
      set_color(BR_YELLOW);
      printArrayCol(4,70,array,ARRAY_SIZE/2);
      set_color(BR_GREEN);
      printArrayCol(5+ARRAY_SIZE/4,70,array,ARRAY_SIZE*3/4);
      
      set_color(CYAN);
      printArrayCol(4,60,copy,0);
      set_color(MAGENTA);
      printArrayCol(5+ARRAY_SIZE/4,60,copy,ARRAY_SIZE/4);
      set_color(YELLOW);
      printArrayCol(4,75,copy,ARRAY_SIZE/2);
      set_color(GREEN);
      printArrayCol(5+ARRAY_SIZE/4,75,copy,ARRAY_SIZE*3/4);
      
      mutex_unlock(screem);
      yield();
   }
}

int main(int argc, char **argv){
   system_t * sys;
   uint8_t *array;
   
   signals = malloc(sizeof(signals_t));
   init_signals(signals);
   
   array = malloc(ARRAY_SIZE);
   copy = malloc(ARRAY_SIZE);
   init_array(array);

   screem = malloc(sizeof(mutex_t));
   mutex_init(screem);

   serial_init();
   clear_screen();
   sys = os_init_noMain();

   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("sort", (uint16_t) &st_sort, array, SORT_TS);
   create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   create_thread("display", (uint16_t) &displayMain, array, DISP_TS);
   create_thread("sort spawned", (uint16_t) &mt_sort, array, SORT_TS);
   create_thread("sort spawned", (uint16_t) &mt_sort, array, SORT_TS);
   create_thread("sort spawned", (uint16_t) &mt_sort, array, SORT_TS);
   //EXTRA 5th on no used testing bug
   create_thread("sort spawned", (uint16_t) &mt_sort, array, SORT_TS);
   sys->threadCount--;
   
   sys->threadCount--;
   //sys->threads[sys->threadCount].thread_status=10; 
   sys->threadCount--;
   //sys->threads[sys->threadCount].thread_status=10; 
   sys->threadCount--;
   //sys->threads[sys->threadCount].thread_status=10; 
   
   os_start();
   //should not return here
}

