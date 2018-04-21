#include "globals.h"

int currentDelay = 1000;

void delays(int n){
   while(n){
      _delay_ms(20);
      n-=20;
   }
}

void set_output(){
   //set bit 7 to output
   asm volatile("ldi r31, 0");
   asm volatile("ldi r30, 0x24");
   
   asm volatile("ld  r18, Z");
   asm volatile("ori r18, 0x80");
   asm volatile("st    Z, r18");
}

void LED_on(){
   asm volatile("ldi r31, 0");
   asm volatile("ldi r30, 0x25");
   asm volatile("ld  r18, Z");
   asm volatile("ori r18, 0x80");
   asm volatile("st    Z, r18");
}

void LED_off(){
   asm volatile("ldi  r31, 0");
   asm volatile("ldi  r30, 0x25");
   asm volatile("ld   r18, Z");
   asm volatile("andi r18, 0x7f");
   asm volatile("st     Z, r18");
}

void blinkLEDMain(uint16_t *delay){
   set_output();
   
   while(1){
      LED_on();
      delays(currentDelay);
      LED_off();
      delays(currentDelay);
   }
}

