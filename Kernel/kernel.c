#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <videoDriver.h>
#include <idtLoader.h>
#include <syscalls.h>
#include <registers.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <semaphoreManager.h>
#include <pipe.h>
#include <globals.h>

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
#define SHELL_CODE_START ((void *)0xA00000)

// Eliminamos las declaraciones extern ya que las definimos arriba
// extern void *USERLAND_CODE_ADDRESS;
// extern void *USERLAND_DATA_ADDRESS;
extern void start_userland();

typedef int (*EntryPoint)();

extern int idle_process(int argc, char **argv);
extern void _sti();

void clearBSS(void *bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void *getStackBase()
{
	return (void *)((uint64_t)&endOfKernel + STACK_SIZE // The size of the stack itself, 32KiB
									- sizeof(uint64_t)									// Begin at the top of the stack
	);
}

void *initializeKernelBinary()
{
	void *moduleAddresses[] = {
			SHELL_CODE_START,
	};
	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}

// Conditional global allocation based on selected memory manager
#ifdef FIRSTFIT
KHEAPLCAB kernel_heap;
#endif

#ifdef BUDDY
zone_t buddy_zone;
static page_t buddy_pages[2048];
#endif

void initializeMemoryManagers()
{
	uintptr_t heapStart = (uintptr_t)getStackBase();
	uintptr_t heapEnd = (uintptr_t)SHELL_CODE_START;
	uintptr_t totalSize = heapEnd - heapStart;

	mm_init(heapStart, totalSize);
}

int main()
{
	load_idt();
	initializeMemoryManagers();

	// Initialize scheduler
	scheduler_init();

	// Initialize semaphore manager
	semaphore_manager_init();

	// Initialize pipe manager
	pipe_manager_init();

	// Create IDLE process (PID 0)
	int16_t default_fds[3] = {STDIN, STDOUT, STDERR};
	create_process(idle_process, NULL, "idle", 0, default_fds, 1);

	// Create shell as process (PID 1)
	EntryPoint entryPoint = (EntryPoint)SHELL_CODE_START;
	create_process((MainFunction)entryPoint, NULL, "shell", 2, default_fds, 0);

	// Enable interrupts and start scheduling
	_sti();
	yield();  // Start first context switch

	// Should never reach here
	return 0;
}
