#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

typedef struct
{
  // Registros de propósito general (pusheados por pushState)
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rbp;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;

  // Registros pusheados automáticamente por el hardware durante la interrupción
  uint64_t rip;    // Instruction Pointer
  uint64_t cs;     // Code Segment
  uint64_t rflags; // Flags Register
  uint64_t rsp;    // Stack Pointer (valor original antes de la interrupción)
  uint64_t ss;     // Stack Segment
} registers_t;

#endif