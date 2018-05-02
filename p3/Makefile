HEADERS = os.h synchro.h printThreads.h serial.h
CFILES  = os.c synchro.c printThreads.c serial.c

#Change this variable to point to your Arduino device
#Mac - it may be different
#DEVICE = /dev/tty.usbmodem1431

#Linux (/dev/ttyACM0 or possibly /dev/ttyUSB0)
#DEVICE = /dev/ttyACM0 

#Windows
DEVICE = COM10 

#default target to compile the code
default: p3a

p3a: program3a program
p3b: program3b program

%: %.c $(HEADERS) $(CFILES)
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
#OUT OF DATE
handinLuke: *.c *.h Makefile
	scp $^ calpoly3:~/cpe453/p2_threads
	ssh calpoly3 "cd ~/cpe453/p2_threads; handin jseng 453_program2_7 *"


handinJohn:
	scp *.c *.h Makefile jmthomse@unix3.csc.calpoly.edu:~/cpe453/p/2
	ssh jmthomse@unix3.csc.calpoly.edu "cd ~/cpe453/p/2; handin jseng 453_program2_7 *"


