#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "globals.h"
#include "list.h"

#define PROCESS_STACK_SIZE 4096

typedef struct Process
{
    uint16_t pid;
    uint16_t parent_pid;
    void *stack_base;
    void *stack_pos;
    char **argv;
    char *name;
    uint8_t priority;
    ProcessStatus status;
    int16_t file_descriptors[3];
    int32_t return_value;
    uint8_t unkillable;

    uint16_t quantum_consumed_count;
    uint8_t last_quantum_used;
    uint8_t quantum_usage_percent;
    uint8_t is_io_bound;

    uint16_t waiting_for_pid;
    List zombie_children;
} Process;

int8_t init_process(Process *process, uint16_t pid, uint16_t parent_pid,
                    MainFunction code, char **args, char *name,
                    uint8_t priority, int16_t fds[3], uint8_t unkillable);
void free_process(Process *process);

#endif
