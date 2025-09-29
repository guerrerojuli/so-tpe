#ifndef __sound__h__
#define __sound__h__

#include <stdint.h>

// Implementadas en soundDriver.asm
void play_sound(uint32_t nFrequence);
void stop_sound();

// Implementada en soundDriver.c
void beep(uint64_t freq, uint64_t milis);

#endif