/* Written: Luke Thompson and John Thomsen */

#include <stdlib.h>
#include "synchro.h"
#include "printThreads.h"

#define ARRAY_SIZE 128
#define MAX_VAL 255

#define SORT_TS 100
#define DISP_TS 128

typedef struct sortData {

} sortData;

void init_toSort(uint8_t *toSort){
   uint16_t i;
   for(i=0;i<ARRAY_SIZE;i++){
      toSort[i] = rand()%MAX_VAL;
   }
}

void mt_sort()

int main(int argc, char **argv){
   system_t * sys;
   uint8_t *toSort;
   
   toSort = malloc(ARRAY_SIZE);
   init_toSort(toSort);

   sys = os_init();

   create_thread("sort", (uint16_t) &mt_sort, toSort, TS_SORT);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("display", (uint16_t) &setLEDMain, &delay, SET_LED_SIZE);
   
   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

