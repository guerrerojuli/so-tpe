include ../Makefile.inc

MODULE=shell.bin
LIBC_DIR=../libc
TESTS_DIR=../tests
SOURCES=$(shell find . -type f -name "*.c" -not -name "_loader.c")
ASM_SOURCES=$(shell find . -name "*.asm")
HEADERS=$(shell find . -name "*.h")
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)
LIBC_OBJECTS=$(wildcard $(LIBC_DIR)/src/*.o)
TEST_OBJECTS=$(wildcard $(TESTS_DIR)/src/*.o)

all: $(MODULE)

$(MODULE): $(HEADERS) $(SOURCES) $(ASM_OBJECTS) $(LIBC_OBJECTS) $(TEST_OBJECTS)
	$(GCC) $(GCCFLAGS) -I$(LIBC_DIR)/include -I$(TESTS_DIR)/include -fno-builtin -T shell.ld _loader.c $(SOURCES) $(ASM_OBJECTS) $(LIBC_OBJECTS) $(TEST_OBJECTS) -o ../$(MODULE)
	$(GCC) $(GCCFLAGS) -no-pie -I$(LIBC_DIR)/include -I$(TESTS_DIR)/include -fno-builtin -T shell.ld -Wl,--oformat=elf64-x86-64 _loader.c $(SOURCES) $(ASM_OBJECTS) $(LIBC_OBJECTS) $(TEST_OBJECTS) -o ../shell.elf

%.o: %.asm
	nasm -felf64 $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean print
