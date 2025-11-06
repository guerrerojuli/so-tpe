include ../Makefile.inc

MODULE=shell.bin
LIBC_DIR=../libc
SOURCES=$(shell find . -type f -name "*.c" -not -name "_loader.c") $(shell find ../SampleCodeModule/tests -name "*.c" 2>/dev/null)
ASM_SOURCES=$(shell find . -name "*.asm")
HEADERS=$(shell find . -name "*.h")
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)
LIBC_OBJECTS=$(wildcard $(LIBC_DIR)/src/*.o)

all: $(MODULE)

$(MODULE): $(HEADERS) $(SOURCES) $(ASM_OBJECTS) $(LIBC_OBJECTS)
	$(GCC) $(GCCFLAGS) -I$(LIBC_DIR)/include -I../SampleCodeModule/include -fno-builtin -T shell.ld _loader.c $(SOURCES) $(ASM_OBJECTS) $(LIBC_OBJECTS) -o ../$(MODULE)

%.o: %.asm
	nasm -felf64 $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean print
