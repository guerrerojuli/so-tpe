#include <time.h>
#include <stdint.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

void delay(uint64_t milis) {
	uint64_t start = ticks_elapsed();
	while(ticks_elapsed() - start < (18 * milis) / 1000);
}
