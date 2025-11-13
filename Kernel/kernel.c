// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


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
#include <semaphores.h>
#include <pipe.h>
#include <globals.h>
#include <keyboardDriver.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;
#define STACK_PAGES 8
#define STACK_SIZE (PageSize * STACK_PAGES)

#define SHELL_CODE_START ((void *)0xA00000)

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
	return (void *)((uint64_t)&endOfKernel + STACK_SIZE - sizeof(uint64_t));
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

	scheduler_init();

	semaphore_manager_init();

	init_keyboard();

	pipe_manager_init();

	int16_t default_fds[3] = {STDIN, STDOUT, STDERR};
	create_process(idle_process, NULL, "idle", 0, default_fds, 1);

	EntryPoint entryPoint = (EntryPoint)SHELL_CODE_START;
	create_process((MainFunction)entryPoint, NULL, "shell", 2, default_fds, 0);

	_sti();
	yield();

	return 0;
}
