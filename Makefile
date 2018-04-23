#Change this variable to point to your Arduino device
#Mac - it may be different
#DEVICE = /dev/tty.usbmodem1431

#Linux (/dev/ttyACM0 or possibly /dev/ttyUSB0)
#DEVICE = /dev/ttyACM0 

#Windows
DEVICE = COM10 

#default target to compile the code
default: main

1: lab1_part1 program
2: lab1_part2 program
3: lab1_part3 program
p2: program2 program


%: %.c serial.c blinkLED.c program2.h printThreads.c
	avr-gcc -g -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf $^
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#flash the Arduino with the program
program: main.hex
	#Mac
	avrdude -D -pm2560 -P $(DEVICE) -c wiring -F -u -U flash:w:main.hex

#remove build files
clean:
	rm -fr *.elf *.hex *.o

#luke stuff
handin:
	scp *.c *.h Makefile jmthomse@unix3.csc.calpoly.edu:~/cpe453/l/1
	ssh jmthomse@unix3.csc.calpoly.edu "cd ~/cpe453/l/1; handin jseng 453_lab1_7 *"


