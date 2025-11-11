

#ifndef INTERRUPS_H_
#define INTERRUPS_H_

#include <idtLoader.h>

void _irq00Handler(void);
void _irq01Handler(void);

void _int80Handler(void);
void _yieldHandler(void);

void _exception0Handler(void);
void _exception6Handler(void);

void _cli(void);

void _sti(void);

void _hlt(void);

void picMasterMask(uint8_t mask);

void picSlaveMask(uint8_t mask);

void haltcpu(void);

void *_initialize_stack_frame(void *wrapper, void *code, void *stack_top, void *args);

#endif
