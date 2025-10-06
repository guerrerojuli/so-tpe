#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "globals.h"

#define PROCESS_STACK_SIZE 4096  // 4KB per process

// Process Control Block (PCB)
typedef struct Process {
    uint16_t pid;
    uint16_t parent_pid;
    void *stack_base;          // Allocated from buddy
    void *stack_pos;           // Current RSP
    char **argv;               // Process arguments
    char *name;                // Process name
    uint8_t priority;          // 0-4
    ProcessStatus status;
    int16_t file_descriptors[3];  // stdin, stdout, stderr
    int32_t return_value;
    uint8_t unkillable;        // For IDLE and critical processes
} Process;

// Process management
void init_process(Process *process, uint16_t pid, uint16_t parent_pid,
                  MainFunction code, char **args, char *name,
                  uint8_t priority, int16_t fds[3], uint8_t unkillable);
void free_process(Process *process);

#endif
