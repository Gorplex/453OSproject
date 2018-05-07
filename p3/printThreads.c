/* Written: Luke Thompson and John Thomsen */
#include "printThreads.h"
struct mutex_t * screem;		/* screen mutex */
void printThread(thread_t thread) {

   set_color(RED);
   print_string("\r\n\tThread Name:\t"); set_color(BR_RED);     print_string(thread.name);
   set_color(GREEN);
   print_string("\r\n\tThread PC:\t");   set_color(BR_GREEN);   print_hex(thread.pc);
   set_color(YELLOW);
   print_string("\r\n\tStack Usage:\t"); set_color(BR_YELLOW);  print_int(thread.stackEnd-thread.stackPtr);
   set_color(BLUE);
   print_string("\r\n\tStack Size:\t");  set_color(BR_BLUE);    print_int(thread.stackEnd-thread.stackBase);
   set_color(MAGENTA);
   print_string("\r\n\tCur StackPtr:\t"); set_color(BR_BLUE);  set_color(BR_MAGENTA); print_hex(thread.stackPtr);
   set_color(CYAN);
   print_string("\r\n\tStack Base:\t");  set_color(BR_CYAN);    print_hex(thread.stackBase);
   set_color(WHITE);
   print_string("\r\n\tStack End:\t");   set_color(BR_WHITE);   print_hex(thread.stackEnd);
   set_color(RED);
   print_string("\r\n\tThread Status:\t");   set_color(BR_RED);   
   //extra spacing to overwrite longer words
   switch(thread.thread_status){
      case THREAD_RUNNING:
      print_string("THREAD RUNNING "); 
      break;
      case THREAD_READY:
      print_string("THREAD READY   "); 
      break;
      case THREAD_SLEEPING:
      print_string("THREAD SLEEPING"); 
      break;
      case THREAD_WAITING:
      print_string("THREAD WAITING "); 
      break;
      default:
      print_string("UNRECOGNIZED   "); 
      break;
   }
   set_color(GREEN);
   print_string("\r\n\tWakeup Time:\t");   set_color(BR_GREEN);   print_hex(thread.wakeup_time);
   set_color(YELLOW);
   print_string("\r\n\tSchedule Count: ");   set_color(BR_YELLOW);   print_hex(thread.sched_count);
   print_string("\r\n");
} 

void printSys(system_t * sys) {
   mutex_lock(screem);
   set_cursor(1,1);
   set_color(CYAN);
   print_string("Program 3\r\n");
   print_string("System Time: ");               print_int(sys->time);
   print_string("\r\nNumber of Threads: ");     print_int(sys->threadCount);

   print_string("\r\n\r\n");
   int i;
   for(i=0; i < sys->threadCount; i++) {
      set_color(CYAN);
      print_string("\r\nThread ID: ");    print_int(i);
      printThread( sys->threads[i] );
   }
   mutex_unlock(screem);
}

void printThreadsMain(uint16_t * sys){
   
   serial_init();
   clear_screen();
   while(1) {
      printSys( (system_t *) sys);
   }
}
 
