#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
// Números de Syscall
#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_CLEAR_TEXT_BUFFER 7

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);

uint64_t sys_clear_text_buffer_wrapper(void);

#endif