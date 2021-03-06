/* Written: Luke Thompson and John Thomsen */
#ifndef SERIAL
#define SERIAL

#include <stdint.h>
#include <avr/io.h>

#define OFFSET_DIG 48
#define OFFSET_HEX 55
#define ESC 0x1B

#define RESET       0

#define RED         31
#define GREEN       32
#define YELLOW      33
#define BLUE        34
#define MAGENTA     35
#define CYAN        36
#define WHITE       37


#define BR_BLACK        90
#define BR_RED          91
#define BR_GREEN        92
#define BR_YELLOW       93
#define BR_BLUE         94
#define BR_MAGENTA      95
#define BR_CYAN         96
#define BR_WHITE        97

void serial_init();
uint8_t byte_available();
uint8_t read_byte();
uint8_t write_byte(uint8_t b);

uint16_t print_string(char* s);                  //print a string return length
void print_int(uint16_t i);                  //print an 8-bit or 16-bit unsigned integer
void print_int32(uint32_t i);                //print a 32-bit unsigned integer
void print_int_padded(uint16_t i);           //print an 8-bit or 16-bit unsigned integer with leading 0s
void print_int32_padded(uint32_t i);        //print a 32-bit unsigned integer with leading 0s
void print_hex(uint16_t i);                  //print an 8-bit or 16-bit unsigned integer in hex format
void print_hex32(uint32_t i);                //print a 32-bit unsigned integer in hex format
void print_hex_raw(uint16_t i);              //no leading 0x
void set_cursor(uint8_t row, uint8_t col);   //set the cursor position
void set_color(uint8_t color);               //set the foreground color
void clear_screen();                         //erase all text on the screen

void print_int_spaces(uint16_t i);           //print an 8-bit or 16-bit unsigned integer with leading spaces
void print_int32_spaces(uint32_t i);         //print a 32-bit unsigned integer with leading spaces
uint16_t print_labeled_int(uint8_t row, uint8_t col, char *label, uint16_t i);
uint16_t print_labeled_int32(uint8_t row, uint8_t col, char *label, uint16_t i);

void set_cursor_home();

#endif
