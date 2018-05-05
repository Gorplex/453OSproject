/* Written: Luke Thompson and John Thomsen */
#ifndef PRINT_THREAD_H
#define PRINT_THREAD_H

#include "os.h"

typedef struct queue_t {
   TID_T q[MAX_THREADS];
   uint16_t start;
   uint16_t size;
} queue_t;

typedef struct mutex_t {
   uint8_t locked;
   TID_T owner;
   queue_t queue;
} mutex_t;

void mutex_init(struct mutex_t* m);
void mutex_lock(struct mutex_t* m);
void mutex_unlock(struct mutex_t* m);

typedef struct semaphore_t {
   uint16_t keys;
   queue_t queue;
} semaphore_t;

void sem_init(struct semaphore_t* s, int8_t value);
void sem_wait(struct semaphore_t* s);
void sem_signal(struct semaphore_t* s);
void sem_signal_swap(struct semaphore_t* s);
#endif




