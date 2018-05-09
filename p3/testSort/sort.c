/* Written: Luke Thompson and John Thomsen */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define INDEX uint8_t
#define ARRAY_SIZE 128
#define MAX_VAL 255
#define NUM_THREADS 4   //number of sorting threads

uint8_t * copy;         //storage array

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
   //printf("Start: %d End: %d\n", start, end); 
   if(end-start > 1){
      mid = (end-start)/2+start;
      //printf("Mid: %d\n", mid); 
      sort(sorted, start, mid, array);
      sort(sorted, mid, end, array);
      merge(array, start, mid, end, sorted);
   }
}

int main(int argc, char **argv){
   uint8_t *array;
   INDEX i;
   
   array = malloc(ARRAY_SIZE);
   copy = malloc(ARRAY_SIZE);
   init_array(array);
   
   for(i=0;i<ARRAY_SIZE;i++){
      copy[i]=array[i];
   }
   
   printf("here\n");

   sort(copy, 0, ARRAY_SIZE, array);
   
   printf("done\n");

   for(i=0;i<ARRAY_SIZE;i++){
      printf("%d: %d\n", i, array[i]);
   }
}

