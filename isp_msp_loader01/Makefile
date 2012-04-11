

COPS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding 



AOPS = --warn --fatal-warnings

OOPS = -std-compile-opts -strip-debug -disable-simplify-libcalls

LOPS = -Wall -m32 -emit-llvm -disable-simplify-libcalls

HLOPS = -Wall -emit-llvm 

LLCOPS = -march=msp430

all : out.hex

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
	rm -f dumphex


startup.o : startup.s
	msp430-as startup.s -o startup.o

reset.o : reset.s
	msp430-as reset.s -o reset.o


mspavrloader.o : mspavrloader.c blinker.h
	msp430-gcc $(COPS) -c mspavrloader.c -o mspavrloader.o


mspavrloader.elf : memmap startup.o reset.o mspavrloader.o
	msp430-ld -T memmap reset.o mspavrloader.o startup.o -o mspavrloader.elf

out.hex : mspavrloader.elf
	msp430-objdump -D mspavrloader.elf > mspavrloader.list
	msp430-objcopy -O ihex mspavrloader.elf out.hex



dumphex : dumphex.c
	gcc -o dumphex dumphex.c

blinker.h : blinker02.s.hex dumphex
	./dumphex blinker02.s.hex


blinker01.s.hex : blinker01.s
	avra -fI blinker01.s

blinker02.s.hex : blinker02.s
	avra -fI blinker02.s




