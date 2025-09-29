include Makefile.inc

all: libc shell

libc:
	cd libc; make

shell: libc
	cd shell; make

clean:
	cd libc; make clean
	cd shell; make clean
	rm -rf *.bin

.PHONY: libc shell all clean
