

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