/* Written: Luke Thompson and John Thomsen */

#include "os.h"
#include "printThreads.h"

int main(int argc, char **argv){
   system_t * sys;
   
   sys = os_init();

   //create_thread("blink", (uint16_t) &blinkLEDMain, &delay, BLINK_LED_SIZE);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   //create_thread("setLED", (uint16_t) &setLEDMain, &delay, SET_LED_SIZE);
   
   os_start();
   sei();      //just to be sure
   while(1){
      sei();   //just to be extra sure
   }
}

