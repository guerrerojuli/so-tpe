include Makefile.inc

all: libc tests shell

libc:
	cd libc; make

tests:
	cd tests; make

shell: libc tests
	cd shell; make

clean:
	cd libc; make clean
	cd tests; make clean
	cd shell; make clean
	rm -rf *.bin *.elf

.PHONY: libc tests shell all clean
