
#ARMGNU=arm-thumb-elf
#ARMGNU=arm-none-linux-gnueabi
ARMGNU?=arm-none-eabi

LOPS = -Wall -m32 -emit-llvm

LLCOPS = -march=thumb

COPS = -Wall  -O2 -nostdlib -nostartfiles -ffreestanding

AOPS = --warn --fatal-warnings

OOPS = -std-compile-opts

stuff = -O2 -std-compile-opts -strip-debug



all : pdi_loader01.gcc.bin pdi_loader01.clang.norm.bin pdi_loader01.clang.opt.bin

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

pdi_loader01.gcc.o : pdi_loader01.c blinker.h
	$(ARMGNU)-gcc $(COPS) -mthumb -c pdi_loader01.c -o pdi_loader01.gcc.o

pdi_loader01.gcc.bin : memmap vectors.o pdi_loader01.gcc.o
	$(ARMGNU)-ld -o pdi_loader01.gcc.elf -T memmap vectors.o pdi_loader01.gcc.o
	$(ARMGNU)-objdump -D pdi_loader01.gcc.elf > pdi_loader01.gcc.list
	$(ARMGNU)-objcopy pdi_loader01.gcc.elf pdi_loader01.gcc.bin -O binary


pdi_loader01.clang.bc : pdi_loader01.c blinker.h
	clang $(LOPS) -c pdi_loader01.c -o pdi_loader01.clang.bc

pdi_loader01.clang.norm.bin : memmap vectors.o pdi_loader01.clang.bc
	llc $(LLCOPS) pdi_loader01.clang.bc -o pdi_loader01.clang.norm.s
	$(ARMGNU)-as pdi_loader01.clang.norm.s -o pdi_loader01.clang.norm.o
	$(ARMGNU)-ld -o pdi_loader01.clang.norm.elf -T memmap vectors.o pdi_loader01.clang.norm.o
	$(ARMGNU)-objdump -D pdi_loader01.clang.norm.elf > pdi_loader01.clang.norm.list
	$(ARMGNU)-objcopy pdi_loader01.clang.norm.elf pdi_loader01.clang.norm.bin -O binary

pdi_loader01.clang.opt.bin : memmap vectors.o pdi_loader01.clang.bc
	opt $(OOPS) pdi_loader01.clang.bc -o pdi_loader01.clang.opt.bc
	llc $(LLCOPS) pdi_loader01.clang.opt.bc -o pdi_loader01.clang.opt.s
	$(ARMGNU)-as pdi_loader01.clang.opt.s -o pdi_loader01.clang.opt.o
	$(ARMGNU)-ld -o pdi_loader01.clang.opt.elf -T memmap vectors.o pdi_loader01.clang.opt.o
	$(ARMGNU)-objdump -D pdi_loader01.clang.opt.elf > pdi_loader01.clang.opt.list
	$(ARMGNU)-objcopy pdi_loader01.clang.opt.elf pdi_loader01.clang.opt.bin -O binary

dumphex : dumphex.c
	gcc -o dumphex dumphex.c

blinker.h : blinker03.hex dumphex
	./dumphex blinker03.hex


blinker01.s.hex : blinker01.s
	avra -fI blinker01.s

blinker02.s.hex : blinker02.s
	avra -fI blinker02.s

blinker03.hex : blinker03.s xblinker.c avrmap
	avr-as blinker03.s -o blinker03.o
	avr-gcc -c $(COPS) xblinker.c -o xblinker.o
	avr-ld -T avrmap blinker03.o xblinker.o -o blinker03.elf
	avr-objdump -D blinker03.elf > blinker03.list
	avr-objcopy blinker03.elf -O ihex blinker03.hex

clean:
	rm -f *.bin
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.bc
	rm -f *.opt.s
	rm -f *.norm.s
	rm -f blinker01.s.*
	rm -f blinker02.s.*
	rm -f dumphex
	rm -f *.hex


