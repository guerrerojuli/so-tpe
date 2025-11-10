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
#define SYSCALL_MALLOC 9
#define SYSCALL_FREE 10
#define SYSCALL_MEM_STATE 11
#define SYSCALL_SEM_INIT 12
#define SYSCALL_SEM_OPEN 13
#define SYSCALL_SEM_CLOSE 14
#define SYSCALL_SEM_DESTROY 15
#define SYSCALL_SEM_WAIT 16
#define SYSCALL_SEM_POST 17
#define SYSCALL_WAITPID 18
#define SYSCALL_PIPE_OPEN 19
#define SYSCALL_PIPE_CLOSE 20
#define SYSCALL_PIPE_GET 21
#define SYSCALL_GET_PROCESS_INFO 22
#define SYSCALL_SLEEP 23
#define SYSCALL_UNBLOCK 24
#define SYSCALL_GET_TICKS 25

// I/O syscalls
uint64_t sys_read(uint64_t fd, uint64_t buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3);
uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3);
uint64_t sys_clear_text_buffer_wrapper(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);

// Process management syscalls
uint64_t sys_create_process(uint64_t code_ptr, uint64_t args_ptr, uint64_t name_ptr, uint64_t priority, uint64_t fds_ptr, uint64_t _unused);
uint64_t sys_kill_process(uint64_t pid, uint64_t retval, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_get_pid(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);
uint64_t sys_yield(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);
uint64_t sys_set_priority(uint64_t pid, uint64_t new_priority, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_block(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_unblock(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_waitpid(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_sleep(uint64_t seconds, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);

// Memory management syscalls
uint64_t sys_malloc(uint64_t size, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_free(uint64_t ptr, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_mem_state(uint64_t total_ptr, uint64_t free_ptr, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);

// Semaphore syscalls
uint64_t sys_sem_init(uint64_t sem_id, uint64_t initial_value, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_sem_open(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_sem_close(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_sem_destroy(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_sem_wait(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_sem_post(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);

// Pipe syscalls
uint64_t sys_pipe_open(uint64_t id, uint64_t mode, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);
uint64_t sys_pipe_close(uint64_t id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5);
uint64_t sys_pipe_get(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);

// Process info syscalls
uint64_t sys_get_process_info(uint64_t info_array_ptr, uint64_t max_count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4);

// Time syscalls
uint64_t sys_get_ticks(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6);

#endif