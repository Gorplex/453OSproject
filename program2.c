/* Written: Luke Thompson and John Thomsen */

#include <string.h>

#include "globals.h"
#include "os.h"
#include "blinkLED.h"
#include "printThreads.h"


/* sys is now stored in heap space because of weird issue of sys not updating */
system_t * sys;

int main(int argc, char **argv){
   uint16_t delay;
   delay = BLINK_LED_DELAY;

   serial_init();
   clear_screen();
   os_init();

   create_thread("blink", (uint16_t) &blinkLEDMain, &delay, BLINK_LED_SIZE);
   create_thread("stats", (uint16_t) &printThreadsMain, sys, PRINT_THREAD_SIZE);
   create_thread("setLED", (uint16_t) &setLEDMain, &delay, SET_LED_SIZE);
   
   os_start();
}

