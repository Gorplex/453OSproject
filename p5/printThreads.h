/* Written: Luke Thompson and John Thomsen */
#ifndef PRINT_THREAD_H
#define PRINT_THREAD_H
#include "os.h"
#include "serial.h"
#include "synchro.h"

#define PRINT_THREAD_SIZE 1000

void printThread(thread_t thread);
void printSys(system_t * sys);
void printm(uint8_t color, uint8_t row, uint8_t col, char * str);

#endif
