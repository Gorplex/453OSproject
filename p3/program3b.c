/* Written: Luke Thompson and John Thomsen */

#include <stdlib.h>
#include "synchro.h"
#include "printThreads.h"

#define INDEX uint8_t
#define ARRAY_SIZE 128
#define MAX_VAL 255
#define NUM_THREADS 4   //number of sorting threads

#define SORT_TS 200
#define DISP_TS 200

uint8_t * copy;         //storage array
extern struct mutex_t * screem;

typedef struct signals_t {
   mutex_t *initThreads;
   uint8_t numCreated;
   semaphore_t *done;
} signals_t;

signals_t * signals;

void printThreadsMain(uint16_t *sys){
   serial_init();
   clear_screen();
   
   while(1){
      printSys((system_t *) sys);
   }
}

void init_signals(signals_t * s){
   s->initThreads = malloc(sizeof(mutex_t));
   mutex_init(s->initThreads);
   s->numCreated = 1;
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
   if(end-start > 1){
      mid = (end-start)/2;
      sort(sorted, start, mid, array);
      sort(sorted, mid, end, array);
      merge(array, start, mid, end, sorted);
   }
}

void mt_sort(uint8_t *array){
   TID_T myID;
   uint16_t start;
   uint16_t end;
   uint16_t i;

   mutex_lock(signals->initThreads);
   myID = signals->numCreated;
   while(signals->numCreated <= NUM_THREADS){
      create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   }
   if(myID==1){
      copy = malloc(ARRAY_SIZE);
   }
   while(1){
      if(myID==1){
         for(i=0;i<ARRAY_SIZE;i++){
            copy[i]=array[i];
         }
      }
      mutex_unlock(signals->initThreads);
      //ALL START
      start = ARRAY_SIZE*(myID-1)/NUM_THREADS; 
      end = ARRAY_SIZE*(myID)/NUM_THREADS; 
      
      sort(copy, start, end, array); 
      
      sem_signal(signals->done);
      sem_wait(signals->done);
      
      mutex_lock(signals->initThreads); //for restarting
      if(myID==1){
         merge(array, 0, ARRAY_SIZE/4, ARRAY_SIZE/2, copy);
         merge(array, ARRAY_SIZE/2, ARRAY_SIZE*3/4, ARRAY_SIZE, copy);
         merge(copy, 0, ARRAY_SIZE/2, ARRAY_SIZE, array);
         thread_sleep(1000);
         
         //restart
         init_array(array);
         //signal others to wait on mutex
         sem_signal_swap(signals->done);
         sem_signal_swap(signals->done);
         sem_signal_swap(signals->done);
         //reset sem
         sem_init(signals->done, 1-NUM_THREADS);
      }
   }
}

void displayMain(){
   while(1){

   }
}

int main(int argc, char **argv){
   system_t * sys;
   uint8_t *array;
   
   //signals = malloc(sizeof(signals_t));
   //init_signals(signals);
   
   //array = malloc(ARRAY_SIZE);
   //init_array(array);

   screem = malloc(sizeof(mutex_t));
   mutex_init(screem);

   serial_init();
   clear_screen();
   sys = os_init();

   //create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   //create_thread("display", (uint16_t) &displayMain, array, DISP_TS);
   
   os_start();
   //should not return here
   while(1){
      printSys(sys);
   }
}

