#include "printThreads.h"



void printThread(thread_t thread) {
   set_color(RED);
   print_string("\r\nThread Name: "); set_color(BR_RED); print_string(thread.name);
   set_color(GREEN);
   print_string("\r\nThread PC: ");     set_color(BR_GREEN);         print_hex(thread.pc);
   set_color(YELLOW);
   print_string("\r\nStack Usage: ");   set_color(BR_YELLOW);             print_int(thread.stackEnd-thread.stackPtr);
   set_color(BLUE);
   print_string("\r\nStack Size: ");   set_color(BR_BLUE);          print_int(thread.stackEnd-thread.stackBase);
   set_color(MAGENTA);
   print_string("\r\nCurrent top of stack: "); set_color(BR_BLUE);  set_color(BR_MAGENTA); print_hex(thread.stackPtr);
   set_color(CYAN);
   print_string("\r\nStack Base: "); set_color(BR_CYAN);            print_hex(thread.stackBase);
   set_color(WHITE);
   print_string("\r\nStack End: ");   set_color(BR_WHITE);           print_hex(thread.stackEnd);
   print_string("\r\n");
} 

void printSys(system_t * sys) {

   set_cursor(1,1);
   set_color(CYAN);
   print_string("Program 2\r\n");
   print_string("System Time: ");               print_int(sys->time/100);
   print_string("\r\nNumber of Threads: ");     print_int(sys->threadCount);

   print_string("\r\n\r\n");
   int i;
   for(i=0; i < sys->threadCount; i++) {
      set_color(CYAN);
      print_string("\r\nThread ID: ");    print_int(i);
      printThread( sys->threads[i] );
   }
   //_delay_ms(5);
}

void printThreadsMain(uint16_t * sys){
   clear_screen();
   while(1) {

      printSys( (system_t *) sys);
   }
}
 
