#include <scheduler.h>

// Keep scheduler’s internal task table
#define MAX_TASKS 2
static task_t tasks[MAX_TASKS];
static int current_index = -1;

void scheduler_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].state = TASK_UNUSED;
        tasks[i].rsp = 0;
        tasks[i].entry = 0;
    }
    current_index = -1;
}

void scheduler_task_exit(void)
{
    if (current_index >= 0)
    {
        tasks[current_index].state = TASK_FINISHED;
    }
}

static int find_next_ready(int from)
{
    // Non-preemptive: run one task until it finishes; but support manual yield selection
    // Here, we just alternate between available ready tasks.
    for (int i = 1; i <= MAX_TASKS; i++)
    {
        int idx = (from + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_READY)
            return idx;
    }
    return -1;
}

uint64_t scheduler_switch(uint64_t current_rsp)
{
    // If we are currently running a task, save its RSP
    if (current_index >= 0 && tasks[current_index].state == TASK_RUNNING)
    {
        tasks[current_index].rsp = current_rsp;
    }

    int next = find_next_ready(current_index < 0 ? 0 : current_index);
    if (next < 0)
    {
        // No task to run: return 0 to signal no context switch needed
        return 0;
    }

    // Mark current as ready if it was running (cooperative yield)
    if (current_index >= 0 && tasks[current_index].state == TASK_RUNNING)
    {
        tasks[current_index].state = TASK_READY;
    }

    // Switch to next
    current_index = next;
    tasks[current_index].state = TASK_RUNNING;
    return tasks[current_index].rsp;
}

int scheduler_add_task(uint64_t initial_rsp, void (*entry)(void))
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == TASK_UNUSED)
        {
            tasks[i].rsp = initial_rsp;
            tasks[i].entry = entry;
            tasks[i].state = TASK_READY;
            return i;
        }
    }
    return -1;
}



