# HEADERS = os.h synchro.h printThreads.h serial.h WavePinDefs.h SdInfo.h SdReader.h ext2Reader.h
CFILES  =   serial.c SdReader.c ext2Luke.c

#Change this variable to point to your Arduino device
#Mac - it may be different
#DEVICE = /dev/tty.usbmodem1431
#Linux (/dev/ttyACM0 or possibly /dev/ttyUSB0)
#DEVICE = /dev/ttyACM0 
#Windows
#DEVICE = COM10 
#ifeq "$(OS)" "Windows_NT"
#   DEVICE = COM10
#else
#   DEVICE = /dev/ttyACM0 
#endif
WIN = COM10
LNX = /dev/ttyACM0 

#default target to compile the code
default: program5 program



#FOR SENG SPEC
program_5: program5

%: %.c  $(CFILES)
	avr-gcc -g -Wall -pedantic -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o main.elf $^
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#flash the Arduino with the program
program: main.hex
	if [ "$$(uname)" = "MSYS_NT-10.0" ]; \
		then avrdude -D -pm2560 -P $(WIN) -c wiring -F -u -U flash:w:main.hex; \
		else avrdude -D -pm2560 -P $(LNX) -c wiring -F -u -U flash:w:main.hex; \
	fi;
#	avrdude -D -pm2560 -P $(DEVICE) -c wiring -F -u -U flash:w:main.hex


screen:
	screen /dev/ttyACM0 115200

#remove build files
clean:
	rm -fr *.elf *.hex *.o

#luke stuff
#OUT OF DATE
handinLuke: *.c *.h Makefile
	scp $^ calpoly3:~/cpe453/p5_nonOS
	ssh calpoly3 "cd ~/cpe453/p5_nonOS; handin jseng 453_program5_7 *"


handinJohn:
	scp *.c *.h Makefile jmthomse@unix3.csc.calpoly.edu:~/cpe453/p/3
	ssh jmthomse@unix3.csc.calpoly.edu "cd ~/cpe453/p/3; handin jseng 453_program3_7 *"

#works but is local to this
#checkDev:
#	if [ "$$(uname)" = "MSYS_NT-10.0" ]; then DEVICE=COM10;else DEVICE=/dev/tty.usbmodem1431; fi


