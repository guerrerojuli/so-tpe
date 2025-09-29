#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);
void * memmove(void * destination, const void * source, uint64_t length);

char *cpuVendor(char *result);
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
void uint64_to_hex_string(uint64_t num, char *buffer, uint16_t buffer_size);

// Stack canary support
extern uintptr_t __stack_chk_guard;
void __attribute__((noreturn)) __stack_chk_fail(void);

#endif