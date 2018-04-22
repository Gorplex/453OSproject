#include "printThreads.h"



void printThread(thread_t thread) {
   print_string("\r\nThread Name: ");           print_string(thread.name);
   print_string("\r\nThread PC: ");             print_hex(thread.pc);
   print_string("\r\nStack Usage: ");               print_int(thread.stackEnd-thread.stackPtr);
   print_string("\r\nStack Size: ");            print_int(thread.stackEnd-thread.stackBase);
   print_string("\r\nCurrent top of stack: ");  print_hex(thread.stackPtr);
   print_string("\r\nStack Base: ");            print_hex(thread.stackBase);
   print_string("\r\nStack End: ");             print_hex(thread.stackEnd);
   print_string("\r\n");
} 

void printSys(system_t * sys) {

   set_cursor(1,1);
   print_string("Program 2\r\n");
   print_string("System Time: ");               print_int(sys->time/100);
   print_string("\r\nNumber of Threads: ");     print_int(sys->threadCount);

   print_string("\r\n\r\n");
   int i;
   for(i=0; i < sys->threadCount; i++) {
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
 
