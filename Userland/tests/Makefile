include ../Makefile.inc

LIBC_DIR=../libc
SOURCES=$(shell find . -type f -name "*.c")
ASM_SOURCES=$(shell find . -name "*.asm")
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)
C_OBJECTS=$(SOURCES:.c=.o)

all: $(ASM_OBJECTS) $(C_OBJECTS)

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I$(LIBC_DIR)/include -c $< -o $@

clean:
	rm -rf src/*.o

.PHONY: all clean
