#include <registers.h>
#include <consoleDriver.h>
#include <videoDriver.h>
#include <time.h>
#include <interrupts.h>
#include <keyboardDriver.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_EXCEPTION_ID 6

#define SHELL_CODE_START ((void*)0xA00000)
extern void * getStackBase();

// El puntero 'gpr_regs' apunta a la estructura de GPRs guardada por pushState.
// El stack frame de la CPU (con RIP, CS, RFLAGS, RSP_user, SS_user)
// está ubicado en memoria justo DESPUÉS de estos GPRs (en direcciones más altas).
void exceptionDispatcher(int exception, const registers_t *gpr_regs) {
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
		    // Podríamos imprimir el número de excepción si es desconocido
		    console_write("Unknown exception code\n", 23, 0xFFFFFF);
			break;
	}

	print_registers(gpr_regs, 0xFF0000);
	
	console_write("\nException handled. Restarting shell.\n", 39, 0xFFFF00);
	console_write("Press any key to continue...\n", 29, 0x00FF00);
	
	char c;
	while ((c = getChar()) == 0); // Espera bloqueante por una tecla
	
	console_clear();
	
	// Modificar tanto RIP como RSP para reiniciar el shell limpiamente
	registers_t *modifiable_regs = (registers_t *)gpr_regs;
	modifiable_regs->rip = SHELL_CODE_START;  // Apuntar al inicio del shell
	modifiable_regs->rsp = getStackBase();       // Stack limpio para el shell
	
	// Limpiar algunos registros importantes para tener un estado inicial limpio
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
	
	// Ahora retornamos y el flujo normal continúa:
	// popState -> iretq -> ejecuta desde el nuevo RIP con nuevo RSP
	return;
}