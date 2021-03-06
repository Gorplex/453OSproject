/* Written: Luke Thompson and John Thomsen */
#ifndef BLINKLED_H
#define BLINKLED_H

#include <util/delay.h>
#include <stdint.h>

#include "serial.h"

#define BLINK_LED_SIZE 100
#define SET_LED_SIZE 100
#define BLINK_LED_DELAY 500

void blinkLEDMain(uint16_t *delay);
void setLEDMain(uint16_t *delay);

#endif
