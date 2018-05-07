#ifndef PROGRAM3A_H
#define PROGRAM3A_H

#include "os.h"
#include "printThreads.h"
#include "serial.h"
#include "synchro.h"

#define BUF_TS 50
#define PROD_TS 5
#define CONS_TS 5
#define BLINK_TS 0

#define BUF_SIZE 10        //circular queue
#define PROD_DELAY 2000    //ms initial delay for producer
#define CONS_DELAY 1000    //ms initial delay for consumer
#define DELAY_INCREMENT 50  //ms each keypress

#define RAND_RANGE 1000    //posible numbers between 0 and RAND_RANGE-1

typedef struct buffer_t {
   uint16_t prod_delay;
   uint16_t cons_delay;
   uint16_t start;
   uint16_t size;
   uint16_t buf[BUF_SIZE];
   semaphore_t * notEmpty;
   semaphore_t * notFull;
   mutex_t * editing;
} buffer_t;


void display_bounded_buffer(buffer_t *buf);
void producer(buffer_t *buf);
void consumer(buffer_t *buf);

#endif
