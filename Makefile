simon.hex: simon.elf
	avr-objcopy -j .text -j .data -O ihex simon.elf simon.hex

simon.elf: simon.o delay.o overflow.o
	avr-gcc -mmcu=atmega328p simon.o delay.o overflow.o -o simon.elf
	avr-strip simon.elf

delay.o: delay.S
	avr-gcc -mmcu=atmega328p -c delay.S -o delay.o

overflow.o: overflow.S
	avr-gcc -mmcu=atmega328p -c overflow.S -o overflow.o

simon.o: simon.c delay.h
	avr-gcc -mmcu=atmega328p -c simon.c -o simon.o

install:
	avrdude -p atmega328p -c usbtiny -U flash:w:simon.hex:i

clean:
	rm -f *.o *.elf *.lst *.hex
