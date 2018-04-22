#ifndef SERIAL
#define SERIAL

#define RESET       0

#define RED         31
#define GREEN       32
#define BLUE        33
#define YELLOW      34
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

void print_string(char* s);                  //print a string
void print_int(uint16_t i);                  //print an 8-bit or 16-bit unsigned integer
void print_int32(uint32_t i);                //print a 32-bit unsigned integer
void print_int_padded(uint16_t i);           //print an 8-bit or 16-bit unsigned integer with leading 0s
void print_int32i_padded(uint32_t i);        //print a 32-bit unsigned integer with leading 0s
void print_hex(uint16_t i);                  //print an 8-bit or 16-bit unsigned integer in hex format
void print_hex32(uint32_t i);                //print a 32-bit unsigned integer in hex format
void print_hex_raw(uint16_t i);              //no leading 0x
void set_cursor(uint8_t row, uint8_t col);   //set the cursor position
void set_color(uint8_t color);               //set the foreground color
void clear_screen();                         //erase all text on the screen

void set_cursor_home();

#endif
