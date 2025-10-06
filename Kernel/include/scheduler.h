#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <task.h>

void scheduler_init(void);

// Cooperative: pass current RSP, returns next task RSP.
// If return equals input, no switch is required.
uint64_t scheduler_switch(uint64_t current_rsp);

// For tasks to indicate they finished
void scheduler_task_exit(void);

// Register a task built elsewhere (e.g., by process module). Returns task index or -1 on failure.
int scheduler_add_task(uint64_t initial_rsp, void (*entry)(void));

#endif // SCHEDULER_H



