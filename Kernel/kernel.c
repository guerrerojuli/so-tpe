#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <videoDriver.h>
#include <idtLoader.h>
#include <syscalls.h>
#include <registers.h>
#include <soundDriver.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;
#define STACK_PAGES 8
#define STACK_SIZE (PageSize * STACK_PAGES)

// Definimos las direcciones de userland directamente aquí para claridad
// Estas deben coincidir con lo que espera el linker y start_userland.asm
#define SHELL_CODE_START ((void*)0xA00000)
#define MINIGOLF_CODE_START ((void*)0xB00000)

// Eliminamos las declaraciones extern ya que las definimos arriba
// extern void *USERLAND_CODE_ADDRESS;
// extern void *USERLAND_DATA_ADDRESS;
extern void start_userland();

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ STACK_SIZE				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		SHELL_CODE_START,
		MINIGOLF_CODE_START
	};
	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}

int main()
{
	load_idt();

	EntryPoint entryPoint = (EntryPoint)SHELL_CODE_START;
	entryPoint();
	return 0;
}