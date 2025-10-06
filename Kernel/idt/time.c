#include <time.h>
#include <stdint.h>
#include <lib.h>

static unsigned long ticks = 0;

void timer_handler()
{
	ticks++;
}

int ticks_elapsed()
{
	return ticks;
}

int seconds_elapsed()
{
	return ticks / 18;
}

// PIT I/O ports and command bits
#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND      0x43
#define PIT_INPUT_CLOCK  1193182U

void pit_init(uint32_t frequency)
{
    if (frequency == 0)
        frequency = 60;

    uint32_t divisor = PIT_INPUT_CLOCK / frequency;
    if (divisor == 0)
        divisor = 1;

    // Command byte: Channel 0, Access mode lobyte/hibyte, Mode 3 (square wave), Binary
    outb(PIT_COMMAND, 0x36);
    // Send divisor low byte then high byte
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF));
}