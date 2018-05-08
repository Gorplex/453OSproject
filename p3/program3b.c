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

#define SHORT_DELAY 100
#define MED_DELAY 200
#define LONG_DELAY 500

uint8_t * copy;         //storage array
extern struct mutex_t * screem;

typedef struct signals_t {
   mutex_t *initThreads;
   uint8_t numCreated;
   semaphore_t *done;
} signals_t;

signals_t * signals;

void printThreadsMain(uint16_t *sys){
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
   
   thread_sleep(LONG_DELAY);
   mutex_lock(signals->initThreads);
   myID = signals->numCreated;
   while(signals->numCreated <= NUM_THREADS){
      create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
      thread_sleep(MED_DELAY);
   }
   while(1){
      if(myID==1){
         for(i=0;i<ARRAY_SIZE;i++){
            copy[i]=array[i];
         }
         thread_sleep(1000);
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

void printArrayCol(uint16_t row,uint16_t col, uint8_t * array, INDEX index){
   INDEX i;
   for(i=0;i<ARRAY_SIZE/4;i++){
      set_cursor(row+i,col);
      print_int(array[i+index]);
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
   create_thread("sort", (uint16_t) &mt_sort, array, SORT_TS);
   create_thread("display", (uint16_t) &displayMain, array, DISP_TS);
   
   os_start();
   //should not return here
}

