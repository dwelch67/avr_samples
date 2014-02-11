

all : ftdi_avr_loader blinker02.s.hex

clean :
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.bc
	rm -f *.hex
	rm -f *.norm.s
	rm -f *.opt.s
	rm -f blinker01.s.*
	rm -f blinker02.s.*
	rm -f ftdi_avr_loader

ftdi_avr_loader : ftdi_avr_loader.c
	gcc -O2 ftdi_avr_loader.c -o ftdi_avr_loader -lftdi

blinker01.s.hex : blinker01.s
	avra -fI blinker01.s

blinker02.s.hex : blinker02.s
	avra -fI blinker02.s




