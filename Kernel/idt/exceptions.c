// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <registers.h>
#include <consoleDriver.h>
#include <videoDriver.h>
#include <time.h>
#include <interrupts.h>
#include <keyboardDriver.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_EXCEPTION_ID 6

#define SHELL_CODE_START ((void *)0xA00000)
extern void *getStackBase();

void exceptionDispatcher(int exception, const registers_t *gpr_regs)
{
	console_write("\n", 1, 0xFFFFFFF);
	console_write("Exception caught: ", 18, 0xFF0000);
	switch (exception)
	{
	case ZERO_EXCEPTION_ID:
		console_write("Divide by zero exception\n", 25, 0xFFFFFF);
		break;
	case INVALID_OPCODE_EXCEPTION_ID:
		console_write("Invalid opcode exception\n", 25, 0xFFFFFF);
		break;
	default:

		console_write("Unknown exception code\n", 23, 0xFFFFFF);
		break;
	}

	console_write("\nException handled. Restarting shell.\n", 39, 0xFFFF00);
	console_write("Press any key to continue...\n", 29, 0x00FF00);

	char c;
	while ((c = getChar()) == 0)
		;

	console_clear();

	registers_t *modifiable_regs = (registers_t *)gpr_regs;
	modifiable_regs->rip = (uint64_t)SHELL_CODE_START;
	modifiable_regs->rsp = (uint64_t)getStackBase();

	modifiable_regs->rax = 0;
	modifiable_regs->rbx = 0;
	modifiable_regs->rcx = 0;
	modifiable_regs->rdx = 0;
	modifiable_regs->rdi = 0;
	modifiable_regs->rsi = 0;
	modifiable_regs->r8 = 0;
	modifiable_regs->r9 = 0;
	modifiable_regs->r10 = 0;
	modifiable_regs->r11 = 0;
	modifiable_regs->r12 = 0;
	modifiable_regs->r13 = 0;
	modifiable_regs->r14 = 0;
	modifiable_regs->r15 = 0;
	modifiable_regs->rbp = 0;

	return;
}