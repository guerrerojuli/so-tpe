include Makefile.inc

all: libc shell minigolf

libc:
	cd libc; make

shell: libc
	cd shell; make

minigolf: libc
	cd minigolf; make

clean:
	cd libc; make clean
	cd shell; make clean
	cd minigolf; make clean
	rm -rf *.bin

.PHONY: libc shell minigolf all clean
