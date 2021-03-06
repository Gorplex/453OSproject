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
      m->queue.q[(m->queue.start+m->queue.size)%MAX_THREADS] = tid;
      m->queue.size++;
      sys->threads[tid].thread_status=THREAD_WAITING;
      yield();
   }else{
      m->locked = 1;
      m->owner = tid;
      sei();
   }
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
void sem_init(struct semaphore_t* s, int8_t value){
   s->keys = value;
   s->queue.start = 0;
   s->queue.size = 0;
}
void sem_wait(struct semaphore_t* s){
   TID_T tid;
   cli();
   if(s->keys <= 0){
      s->keys--;
      tid = get_thread_id();
      s->queue.q[(s->queue.size+s->queue.start)%MAX_THREADS] = tid;
      s->queue.size++;
      sys->threads[tid].thread_status = THREAD_WAITING;
      yield();
   }else{
      s->keys--;
      sei();
   }
}
void sem_signal(struct semaphore_t* s){
   TID_T tid;
   cli();
   s->keys++; 
   if(s->keys <= 0 && s->queue.size > 0){
      tid = s->queue.q[s->queue.start];
      s->queue.start = (s->queue.start+1)%MAX_THREADS;
      s->queue.size--;
      sys->threads[tid].thread_status = THREAD_READY;
   }
   sei();
}

void sem_signal_swap(struct semaphore_t* s){
   TID_T tid;
   cli();
   s->keys++; 
   if(s->keys <= 0 && s->queue.size > 0){
      tid = s->queue.q[s->queue.start];
      s->queue.start = (s->queue.start+1)%MAX_THREADS;
      s->queue.size--;
      thread_swap(tid);
   }
   sei();
}

