#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

void timer_handler();
int ticks_elapsed();
int seconds_elapsed();

// Initialize PIT to a given frequency (Hz)
void pit_init(uint32_t frequency);

#endif
