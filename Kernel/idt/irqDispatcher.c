// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <registers.h>
#include <time.h>
#include <keyboardDriver.h>

static void (*intHandlers[])(const registers_t *) = {timer_handler, keyboard_handler};

void irqDispatcher(uint64_t irq, const registers_t *registers)
{
    if (irq >= sizeof(intHandlers) / sizeof(intHandlers[0]))
        return;

    intHandlers[irq](registers);
}