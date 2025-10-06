#include <registers.h>
#include <time.h>
#include <keyboardDriver.h>
#include <scheduler.h>

static void (*intHandlers[])(const registers_t *) = {timer_handler, keyboard_handler};

uint64_t irqDispatcher(uint64_t irq, const registers_t *registers)
{
    if (irq >= sizeof(intHandlers) / sizeof(intHandlers[0]))
        return 0;

    // Handle the IRQ (timer, keyboard, etc.)
    intHandlers[irq](registers);

    // Preemptive scheduling on timer tick (IRQ 0)
    if (irq == 0)
    {
        uint64_t current_rsp = (uint64_t)registers;
        return scheduler_switch(current_rsp);
    }

    return 0;
}