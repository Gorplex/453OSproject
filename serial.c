#include <avr/io.h>
#include "globals.h"

#define OFFSET_DIG 48
#define OFFSET_HEX 55
#define ESC 0x1B

/*
 * Initialize the serial port.
 */
void serial_init() {
   uint16_t baud_setting;

   UCSR0A = _BV(U2X0);
   baud_setting = 16; //115200 baud

   // assign the baud_setting
   UBRR0H = baud_setting >> 8;
   UBRR0L = baud_setting;

   // enable transmit and receive
   UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

/*
 * Return 1 if a character is available else return 0.
 */
uint8_t byte_available() {
   return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

/*
 * Unbuffered read
 * Return 255 if no character is available otherwise return available character.
 */
uint8_t read_byte() {
   if (UCSR0A & (1 << RXC0)) return UDR0;
   return 255;
}

/*
 * Unbuffered write
 *
 * b byte to write.
 */
uint8_t write_byte(uint8_t b) {
   //loop until the send buffer is empty
   while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}

   //write out the byte
   UDR0 = b;
   return 1;
}

void print_string(char* s){
    while(*s){
        write_byte((uint8_t)*s);
        s++;
    }
}

void print_int_padded(uint16_t i){
    uint16_t denom = 10000u; 
    while(denom!=0){
        write_byte((uint8_t)(i/denom + OFFSET_DIG));
        i = i%denom;
        denom = denom/10;
    }
}
void print_int(uint16_t i){
   uint16_t denom = 10000u; 
   while(denom!=0){
      if(i/denom!=0 || denom==1){
         write_byte((uint8_t)(i/denom + OFFSET_DIG));
      }
      i = i%denom;
      denom = denom/10;
    }
}

void print_int32_padded(uint32_t i){
    uint32_t denom = 1000000000u; 
    while(denom!=0 || denom==1){
        write_byte((uint8_t)(i/denom + OFFSET_DIG));
        i = i%denom;
        denom = denom/10;
    }
}

void print_int32(uint32_t i){
   uint32_t denom = 1000000000u; 
   while(denom != 0){
      if(i/denom != 0){
         write_byte((uint8_t)(i/denom + OFFSET_DIG));
      }
      i = i%denom;
      denom = denom/10;
   }
}

void print_hex_raw(uint16_t i){
    uint16_t temp;
    uint8_t j=4;
    while(j){
        temp = i >> 12;
        if(temp <= 9){
            write_byte(temp + OFFSET_DIG);
        }else{
            write_byte(temp + OFFSET_HEX);
        }
        i = i<<4;
        j--;
    }
}

void print_hex(uint16_t i){
   print_string("0x");
   print_hex_raw(i);
}

void print_hex32(uint32_t i){
   print_string("0x");
   print_hex_raw(*(((uint16_t *) &i)+1));
   print_hex_raw(*((uint16_t *) &i));
}

void set_cursor(uint8_t row, uint8_t col){
    write_byte(ESC);
    write_byte('[');
    print_int(row);
    write_byte(';');
    print_int(col);
    write_byte('f');
}

void set_color(uint8_t color){
    write_byte(ESC);
    write_byte('[');
    print_int(color);
    write_byte('m');
}

void clear_screen(){
    write_byte(ESC);
    write_byte('[');
    write_byte('2');
    write_byte('J');
}

void set_cursor_home(){
    write_byte(ESC);
    write_byte('[');
    write_byte('H');
}
