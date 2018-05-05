/* Written: Luke Thompson and John Thomsen */
#include "synchro.h"

extern system_t * sys;

void mutex_init(struct mutex_t* m){
   m->locked=0;
   m->owner=0;
   m->queue.start=0;
   m->queue.size=0;
}
void mutex_lock(struct mutex_t* m){
   TID_T tid;
   cli();
   tid=get_thread_id();
   if(m->locked){
      m->queue.q[m->queue.start+m->queue.size] = tid;
      sys->threads[tid].thread_status=THREAD_WAITING;
      yield();
   }else{
      m->locked = 1;
      m->owner = tid;
      sei();
   }
   sei();//CHECK should not need
}
void mutex_unlock(struct mutex_t* m){
   TID_T tid;
   cli();
   if(m->queue.size){
      tid = m->queue.q[m->queue.start];
      m->queue.start = (m->queue.start+1)%MAX_THREADS;
      m->queue.size--;
      m->owner = tid;
      sys->threads[tid].thread_status = THREAD_READY;
   }else{
      m->locked = 0;
      m->owner = 0;
   }
   sei();
}
