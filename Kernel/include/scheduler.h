#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "process.h"
#include "globals.h"

#define MAX_PROCESSES 20
#define IDLE_PID 0
#define NUM_PRIORITIES 5
#define AGING_THRESHOLD 10
#define CALCULATE_QUANTUM(priority) (4 * (1 << (priority)))

typedef struct
{
    Node *processes[MAX_PROCESSES];
    List ready_queues[NUM_PRIORITIES];
    List blocked_queue;
    uint16_t current_pid;
    uint16_t next_unused_pid;
    uint16_t num_processes;
    int16_t remaining_quantum;
    int16_t initial_quantum;
    uint8_t kill_fg_flag;
    uint16_t foreground_pid;
    uint8_t current_priority_level;
} Scheduler;

void scheduler_init();
int16_t create_process(MainFunction code, char **args, char *name,
                       uint8_t priority, int16_t fds[3], uint8_t unkillable);
int32_t kill_process(uint16_t pid, int32_t retval);
int32_t kill_current_process(int32_t retval);
void kill_foreground_process(void);
uint16_t get_pid();
void yield();
int8_t set_priority(uint16_t pid, uint8_t new_priority);
int8_t set_status(uint16_t pid, ProcessStatus new_status);
void *schedule(void *current_rsp);
int32_t waitpid(uint16_t pid);
Process *get_current_process();
Process *get_process_by_pid(uint16_t pid);
uint16_t get_foreground_process_pid();

#endif
