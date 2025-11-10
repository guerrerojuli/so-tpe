#ifndef STDINT_H
#define STDINT_H

// Custom stdint.h for bare-metal environment (no system libc available)
// PVS-Studio: Suppress all warnings about custom type declarations
//-V:uint8_t:677
//-V:uint16_t:677
//-V:uint32_t:677
//-V:uint64_t:677
//-V:uintptr_t:677
//-V:int8_t:677
//-V:int16_t:677
//-V:int32_t:677
//-V:int64_t:677
//-V:size_t:677

// Basic integer types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned long uintptr_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;

typedef unsigned int size_t;

// Stack canary support
extern uintptr_t __stack_chk_guard;
void __attribute__((noreturn)) __stack_chk_fail(void);

#endif 