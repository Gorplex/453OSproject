/* Written: Luke Thompson and John Thomsen */

#include <stdlib.h>
#include "synchro.h"
#include "printThreads.h"

#define INDEX uint8_t
#define ARRAY_SIZE 128
#define MAX_VAL 255
#define NUM_THREADS 4   //number of sorting threads

#define SORT_TS 100
#define DISP_TS 128

signals_t * signals;
uint8_t * copy;         //storage array

typedef struct signals_t {
   mutex_t *initThreads;
   uint8_t numCreated;
   semaphore_t *done;
} signals_t;

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
         sorted[i] = arrary[j];
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
      merge(array, start, end, sorted);
   }
}

void mt_sort(uint8_t *array){
   TID_T myID;
   uint16_t start;
   uint16_t end;

   mutex_lock(signals->initThreads);
   myID = signals->numCreated;
   while(signals->numCreated <= NUM_THREADS){
      create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   }
   copy = malloc(ARRAY_SIZE);
   for(i=0;i<ARRAY_SIZE;i++){
      copy[i]=array[i];
   }
   mutex_unlock(signals->initThreads);
   
   //done creating, include start, not end
   start = ARRAY_SIZE*(myID-1)/NUM_THREADS; 
   end = ARRAY_SIZE*(myID)/NUM_THREADS; 
   
   sort(copy, start, end, array); 
   
   sem_signal(signal->done);
   sem_wait(signal->done);
   
   merge(sorted, 0, ARRAY_SIZE/2, array);
   merge(sorted, ARRAY_SIZE/2, ARRAY_SIZE, array);
   merge(array, 0, ARRAY_SIZE, sorted);
}

int main(int argc, char **argv){
   system_t * sys;
   uint8_t *array;
   
   signals = malloc(sizeof(signals_t));
   init_signals(signals);

   array = malloc(ARRAY_SIZE);
   init_toSort(array);

   sys = os_init_noMain();

   create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("display", (uint16_t) &setLEDMain, &delay, SET_LED_SIZE);
   
   os_start();
   //should not return here
}

