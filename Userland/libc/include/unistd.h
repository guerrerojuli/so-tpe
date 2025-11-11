#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>

#define DEV_NULL 3

typedef enum
{
    READY = 0,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessStatus;

typedef struct
{
    uint16_t pid;
    uint16_t parent_pid;
    char name[64];
    uint8_t priority;
    ProcessStatus status;
    void *stack_base;
    void *stack_pos;
    uint8_t is_foreground;
} ProcessInfo;

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);
uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);
void sys_clear_text_buffer(void);

uint64_t sys_create_process(uint64_t code_ptr, uint64_t args_ptr, uint64_t name_ptr, uint64_t priority, uint64_t fds_ptr);
uint64_t sys_kill_process(uint64_t pid, uint64_t retval);
uint64_t sys_get_pid(void);
uint64_t sys_yield(void);
uint64_t sys_set_priority(uint64_t pid, uint64_t new_priority);
uint64_t sys_block(uint64_t pid);
uint64_t sys_unblock(uint64_t pid);
int64_t sys_waitpid(uint64_t pid);

int64_t sys_sem_init(uint64_t sem_id, uint64_t initial_value);
int64_t sys_sem_open(uint64_t sem_id);
int64_t sys_sem_close(uint64_t sem_id);
int64_t sys_sem_destroy(uint64_t sem_id);
int64_t sys_sem_wait(uint64_t sem_id);
int64_t sys_sem_post(uint64_t sem_id);

int64_t sys_pipe_open(uint16_t id, uint8_t mode);
int64_t sys_pipe_close(uint16_t id);
int16_t sys_pipe_get(void);

int64_t sys_get_process_info(ProcessInfo *info_array, uint64_t max_count);

uint64_t sys_malloc(uint64_t size);
uint64_t sys_free(uint64_t ptr);

uint64_t sys_mem_state(uint64_t total_ptr, uint64_t free_ptr, uint64_t used_ptr, uint64_t name_ptr);

uint64_t sys_sleep(uint64_t seconds);
uint64_t sys_get_ticks(void);

static inline void sleep(int seconds)
{
    sys_sleep((uint64_t)seconds);
}

int64_t create_process_with_fds(void *code, char **args, char *name,
                                uint8_t priority, int16_t fds[3]);

int64_t waitpid(uint16_t pid);

int16_t pipe_get(void);

#endif