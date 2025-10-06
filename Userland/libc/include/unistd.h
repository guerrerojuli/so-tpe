#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>

// I/O syscalls
uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);
uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);
void sys_clear_text_buffer(void);

// Process management syscalls
uint64_t sys_create_process(uint64_t code_ptr, uint64_t args_ptr, uint64_t name_ptr, uint64_t priority, uint64_t fds_ptr);
uint64_t sys_kill_process(uint64_t pid, uint64_t retval);
uint64_t sys_get_pid(void);
uint64_t sys_yield(void);
uint64_t sys_set_priority(uint64_t pid, uint64_t new_priority);
uint64_t sys_block(uint64_t pid);

#endif