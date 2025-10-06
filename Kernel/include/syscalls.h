#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include <globals.h>

// Syscall numbers
#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_CREATE_PROCESS 2
#define SYSCALL_KILL_PROCESS 3
#define SYSCALL_GET_PID 4
#define SYSCALL_YIELD 5
#define SYSCALL_SET_PRIORITY 6
#define SYSCALL_CLEAR_TEXT_BUFFER 7
#define SYSCALL_BLOCK 8

// I/O syscalls
uint64_t sys_read(uint64_t fd, char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3);
uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3);
uint64_t sys_clear_text_buffer_wrapper(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);

// Process management syscalls
uint64_t sys_create_process(uint64_t code_ptr, uint64_t args_ptr, uint64_t name_ptr, uint64_t priority, uint64_t fds_ptr, uint64_t _unused);
uint64_t sys_kill_process(uint64_t pid, uint64_t retval, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_get_pid(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);
uint64_t sys_yield(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);
uint64_t sys_set_priority(uint64_t pid, uint64_t new_priority, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_block(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);

#endif