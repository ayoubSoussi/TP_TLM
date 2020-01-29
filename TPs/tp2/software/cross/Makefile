ifndef CROSS_COMPILE
CROSS_COMPILE=riscv64-unknown-elf-
endif

# Optimization: do not use -O3 as it generates sb/sh/lh/lhu that the *bus*
# is not able to handle yet
# Also, change that so that we can use compressed instructions!
TARGET_CC = $(CROSS_COMPILE)gcc -g -O0 -march=rv32im -mabi=ilp32
TARGET_LD = $(CROSS_COMPILE)ld -nostartfiles -m elf32lriscv
TARGET_OBJDUMP = $(CROSS_COMPILE)objdump
TARGET_READELF = $(CROSS_COMPILE)readelf
HOST_CC = gcc
HOST_LD = ld
HOST_OBJDUMP = objdump
HOST_READELF = readelf

INCLUDE = -I. -I../.. -I/usr/include

OBJS = main.o trap.o it.o boot.o
EXEC = a.out

# Disassembly of the generated executable.
dump.dis: $(EXEC)
	$(TARGET_OBJDUMP) --disassembler-options=no-aliases,numeric -d $< > $@

# Summary of the sections and their size.
sections.txt: $(EXEC)
	$(TARGET_READELF) -S $< > $@


#compiling using the target compiler
main.o: ../main.c ../../address_map.h \
	../../hardware/offsets/gpio.h ../../hardware/offsets/timer.h \
	../../hardware/offsets/vga.h ../../hardware/offsets/uart.h \
	../../hardware/offsets/intc.h hal.h 
	$(TARGET_CC) $(INCLUDE) -c $< -o $@

trap.o : trap.c
	$(TARGET_CC) $(INCLUDE) -c $< -o $@
%.o: %.s
	$(TARGET_CC) -c $<

$(EXEC): $(OBJS) ldscript
	$(TARGET_LD) -Tldscript $(OBJS) -o $@

.PHONY: clean realclean
clean:
	-$(RM) $(OBJS) $(EXEC) dump.dis sections.txt

realclean: clean
	-$(RM) *~
