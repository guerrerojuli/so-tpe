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

static KHEAPLCAB kernel_heap;
static zone_t buddy_zone;
static page_t buddy_pages[2048]; // Array para estructuras de página

void initializeMemoryManagers()
{
	uintptr_t heapStart = (uintptr_t)getStackBase();
	uintptr_t heapEnd = (uintptr_t)SHELL_CODE_START;
	uintptr_t totalSize = heapEnd - heapStart;

	// Entry-based allocator: primera mitad del espacio
	k_heapLCABInit(&kernel_heap);
	k_heapLCABAddBlock(&kernel_heap, heapStart, totalSize / 2);

	// Buddy allocator: segunda mitad del espacio
	uintptr_t buddyStart = heapStart + totalSize / 2;
	uintptr_t buddySize = totalSize / 2;
	uint64_t startPfn = buddyStart / PageSize;
	uint64_t numPages = buddySize / PageSize;

	// Limitar al tamaño del array de páginas
	if (numPages > 2048)
		numPages = 2048;

	buddy_init(&buddy_zone, startPfn, numPages, buddy_pages);
	buddy_add_memory(&buddy_zone, startPfn, numPages);
}

int main()
{
	load_idt();
	initializeMemoryManagers();
	
	// Initialize scheduler (but timer interrupt remains disabled)
	scheduler_init();
	
	// Initialize video system before starting shell
	// Clear both buffers to ensure clean state
	clear_screen(0x000000);
	swap_buffers();

	// Run the shell (timer interrupt disabled, so no preemptive scheduling yet)
	EntryPoint entryPoint = (EntryPoint)SHELL_CODE_START;
	entryPoint();
	return 0;
}
