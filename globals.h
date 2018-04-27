/* Written: Luke Thompson and John Thomsen */
//Global defines

#ifndef GLOBALS_H
#define GLOBALS_H

//place defines and prototypes here
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif
