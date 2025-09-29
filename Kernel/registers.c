#include <registers.h>
#include <consoleDriver.h>
#include <lib.h>

registers_t registers_to_print = {0};

void save_registers(const registers_t *registers)
{
	registers_to_print.rax = registers->rax;
	registers_to_print.rbx = registers->rbx;
	registers_to_print.rcx = registers->rcx;
	registers_to_print.rdx = registers->rdx;
	registers_to_print.rbp = registers->rbp;
	registers_to_print.rdi = registers->rdi;
	registers_to_print.rsi = registers->rsi;
	registers_to_print.r8 = registers->r8;
	registers_to_print.r9 = registers->r9;
	registers_to_print.r10 = registers->r10;
	registers_to_print.r11 = registers->r11;
	registers_to_print.r12 = registers->r12;
	registers_to_print.r13 = registers->r13;
	registers_to_print.r14 = registers->r14;
	registers_to_print.r15 = registers->r15;
	registers_to_print.rip = registers->rip;
	registers_to_print.cs = registers->cs;
	registers_to_print.rflags = registers->rflags;
	registers_to_print.rsp = registers->rsp;
	registers_to_print.ss = registers->ss;
}

// Helper function to print a single register
void print_register(char *name, uint32_t nameDim, uint64_t value, uint32_t color)
{
	char buffer[17];
	uint64_to_hex_string(value, buffer, 17);
	console_write("\t", 1, 0xFFFFFF);
	console_write(name, nameDim, color);
	console_write(buffer, 16, color);
	console_write("\n", 1, 0xFFFFFF);
}

void print_stored_registers()
{
	print_registers(&registers_to_print, 0xFFFFFF);
}

void print_registers(const registers_t *registers, uint32_t color)
{
	print_register("rbp:    ", 8, registers->rbp, color);
	print_register("rax:    ", 8, registers->rax, color);
	print_register("rbx:    ", 8, registers->rbx, color);
	print_register("rcx:    ", 8, registers->rcx, color);
	print_register("rdx:    ", 8, registers->rdx, color);
	print_register("rsi:    ", 8, registers->rsi, color);
	print_register("rdi:    ", 8, registers->rdi, color);
	print_register("r8:     ", 8, registers->r8, color);
	print_register("r9:     ", 8, registers->r9, color);
	print_register("r10:    ", 8, registers->r10, color);
	print_register("r11:    ", 8, registers->r11, color);
	print_register("r12:    ", 8, registers->r12, color);
	print_register("r13:    ", 8, registers->r13, color);
	print_register("r14:    ", 8, registers->r14, color);
	print_register("r15:    ", 8, registers->r15, color);

	// Registros pusheados automáticamente por el hardware durante interrupciones
	print_register("rip:    ", 8, registers->rip, color);
	print_register("cs:     ", 8, registers->cs, color);
	print_register("rflags: ", 8, registers->rflags, color);
	print_register("rsp:    ", 8, registers->rsp, color);
	print_register("ss:     ", 8, registers->ss, color);
}