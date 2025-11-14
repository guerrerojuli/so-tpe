#ifndef STDINT_H
#define STDINT_H

typedef unsigned char uint8_t; //-V677
typedef unsigned short uint16_t; //-V677
typedef unsigned int uint32_t; //-V677
typedef unsigned long uint64_t; //-V677
typedef unsigned long uintptr_t; //-V677

typedef signed char int8_t; //-V677
typedef signed short int16_t; //-V677
typedef signed int int32_t; //-V677
typedef signed long int64_t; //-V677

typedef unsigned int size_t; //-V677

extern uintptr_t __stack_chk_guard;
void __attribute__((noreturn)) __stack_chk_fail(void);

#endif