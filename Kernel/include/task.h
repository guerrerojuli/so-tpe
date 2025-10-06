#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef enum {
    TASK_UNUSED = 0,
    TASK_READY = 1,
    TASK_RUNNING = 2,
    TASK_FINISHED = 3,
} task_state_t;

typedef struct task {
    uint64_t rsp;            // Saved stack pointer (synthetic frame for return path)
    void (*entry)(void);     // Entry function for the task
    task_state_t state;      // Current state
} task_t;

#endif // TASK_H



