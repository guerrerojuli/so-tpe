include ../Makefile.inc

MODULE=minigolf.bin
LIBC_DIR=../libc
INCLUDES=-I$(LIBC_DIR)/include
SOURCES=$(shell find . -type f -name "*.c" -not -name "_loader.c")
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LIBC_OBJECTS_ASM=$(LIBC_DIR)/src/*.o

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM) $(LIBC_OBJECTS_ASM)
	$(GCC) $(GCCFLAGS) $(INCLUDES) -fno-builtin -T minigolf.ld _loader.c $(SOURCES) $(OBJECTS_ASM) $(LIBC_OBJECTS_ASM) -o ../$(MODULE)

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean