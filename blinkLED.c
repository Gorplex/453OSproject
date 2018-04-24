#include "blinkLed.h"

void delays(uint16_t n){
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
   //this print is here to make the code work
   //without this print the LED is solid on
   //optomising out the call to delay??
   print_string("");
   while(1){
      LED_on();
      delays(*delay);
      LED_off();
      delays(*delay);
   }
}

void check_key(uint16_t *delay){
    if(byte_available()){
        switch(read_byte()){
            case 'a':
            *delay-=20;
            if(*delay <= 0){
                *delay = 20;
            }
            break;
            case 'z':
            *delay+=20;
            break;
        }
    }
}

void setLEDMain(uint16_t *delay){
   serial_init(); 
   while(1){
      check_key(delay);   
   }
}
