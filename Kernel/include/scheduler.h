#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "process.h"
#include "globals.h"

#define MAX_PROCESSES 20
#define IDLE_PID 0
#define NUM_PRIORITIES 5
#define AGING_THRESHOLD 100  // Process must consume full quantum this many times before aging

// Process info structure for ps command
typedef struct {
    uint16_t pid;
    uint16_t parent_pid;
    char name[64];
    uint8_t priority;
    ProcessStatus status;
    void *stack_base;
    void *stack_pos;
    uint8_t is_foreground;  // 1 if process is in foreground, 0 otherwise
} ProcessInfo;

// Scheduler API
void scheduler_init();
int16_t create_process(MainFunction code, char **args, char *name,
                       uint8_t priority, int16_t fds[3], uint8_t unkillable);
int32_t kill_process(uint16_t pid, int32_t retval);
int32_t kill_current_process(int32_t retval);
uint16_t get_pid();
void yield();
int8_t set_priority(uint16_t pid, uint8_t new_priority);
int8_t set_status(uint16_t pid, ProcessStatus new_status);
void *schedule(void *current_rsp);  // Called from timer interrupt
int32_t waitpid(uint16_t pid);  // Wait for child process to terminate
int32_t get_process_info(ProcessInfo *info_array, uint32_t max_count);  // Get info for all processes

#endif
