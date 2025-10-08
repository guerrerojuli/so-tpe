// Scheduler implementation - priority-based round-robin with aging

#include <stddef.h>
#include "include/scheduler.h"
#include "include/list.h"
#include "include/memoryManager.h"

typedef struct {
    Node *processes[MAX_PROCESSES];  // Fast PID lookup
    List ready_queues[NUM_PRIORITIES];  // Priority queues
    List blocked_queue;
    uint16_t current_pid;
    uint16_t next_unused_pid;
    uint16_t num_processes;
    int8_t remaining_quantum;
} Scheduler;

static Scheduler scheduler;


void scheduler_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        scheduler.processes[i] = NULL;
    }

    for (int i = 0; i < NUM_PRIORITIES; i++) {
        list_init(&scheduler.ready_queues[i]);
    }

    list_init(&scheduler.blocked_queue);
    scheduler.current_pid = 0;
    scheduler.next_unused_pid = 0;
    scheduler.num_processes = 0;
    scheduler.remaining_quantum = 1;
}

// Find highest priority ready process
static uint16_t get_next_pid() {
    for (int lvl = NUM_PRIORITIES - 1; lvl >= 0; lvl--) {
        if (!list_is_empty(&scheduler.ready_queues[lvl])) {
            Node *node = list_get_first(&scheduler.ready_queues[lvl]);
            Process *process = (Process *)node->data;
            return process->pid;
        }
    }
    return IDLE_PID;
}

int16_t create_process(MainFunction code, char **args, char *name,
                       uint8_t priority, int16_t fds[3], uint8_t unkillable) {
    if (scheduler.num_processes >= MAX_PROCESSES)
        return -1;

    if (priority >= NUM_PRIORITIES)
        priority = NUM_PRIORITIES - 1;

    // Allocate PCB
    Process *process = (Process *)mm_alloc(sizeof(Process));
    init_process(process, scheduler.next_unused_pid, scheduler.current_pid,
                 code, args, name, priority, fds, unkillable);

    // Add to scheduler
    Node *node;
    if (process->pid != IDLE_PID) {
        node = list_append(&scheduler.ready_queues[priority], process);
    } else {
        // IDLE is special - manual node, not in queue
        node = (Node *)mm_alloc(sizeof(Node));
        node->data = process;
        node->prev = node->next = NULL;
    }

    scheduler.processes[process->pid] = node;

    // Find next unused PID
    while (scheduler.processes[scheduler.next_unused_pid] != NULL) {
        scheduler.next_unused_pid = (scheduler.next_unused_pid + 1) % MAX_PROCESSES;
    }

    scheduler.num_processes++;
    return process->pid;
}

int8_t set_priority(uint16_t pid, uint8_t new_priority) {
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL || pid == IDLE_PID)
        return -1;

    if (new_priority >= NUM_PRIORITIES)
        return -1;

    Node *node = scheduler.processes[pid];
    Process *process = (Process *)node->data;

    if (process->status == READY || process->status == RUNNING) {
        // Remove from old queue
        list_remove(&scheduler.ready_queues[process->priority], node);

        // Add to new queue
        node = list_append(&scheduler.ready_queues[new_priority], process);
        scheduler.processes[pid] = node;
    }

    process->priority = new_priority;
    return new_priority;
}

int8_t set_status(uint16_t pid, ProcessStatus new_status) {
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL || pid == IDLE_PID)
        return -1;

    Node *node = scheduler.processes[pid];
    Process *process = (Process *)node->data;
    ProcessStatus old_status = process->status;

    if (new_status == RUNNING || new_status == ZOMBIE || old_status == ZOMBIE)
        return -1;

    if (new_status == old_status)
        return new_status;

    process->status = new_status;

    if (new_status == BLOCKED) {
        // Move to blocked queue
        list_remove(&scheduler.ready_queues[process->priority], node);
        node = list_append(&scheduler.blocked_queue, process);
        scheduler.processes[pid] = node;
    } else if (old_status == BLOCKED && new_status == READY) {
        // Move back to ready with max priority (priority boost)
        list_remove(&scheduler.blocked_queue, node);
        process->priority = NUM_PRIORITIES - 1;
        node = list_prepend(&scheduler.ready_queues[process->priority], process);
        scheduler.processes[pid] = node;
        scheduler.remaining_quantum = 0;  // Force reschedule
    }

    return new_status;
}

int32_t kill_process(uint16_t pid, int32_t retval) {
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL)
        return -1;

    Node *node = scheduler.processes[pid];
    Process *process = (Process *)node->data;

    if (process->status == ZOMBIE || process->unkillable)
        return -1;

    // Remove from queue
    if (process->status == BLOCKED) {
        list_remove(&scheduler.blocked_queue, node);
    } else {
        list_remove(&scheduler.ready_queues[process->priority], node);
    }

    process->status = ZOMBIE;
    process->return_value = retval;

    // Cleanup immediately (simplified - no parent waiting)
    scheduler.processes[pid] = NULL;
    scheduler.num_processes--;
    free_process(process);
    mm_free(node);

    if (pid == scheduler.current_pid) {
        yield();
    }

    return 0;
}

int32_t kill_current_process(int32_t retval) {
    return kill_process(scheduler.current_pid, retval);
}

uint16_t get_pid() {
    return scheduler.current_pid;
}

void yield() {
    scheduler.remaining_quantum = 0;
    __asm__ volatile("int $0x20");  // Force timer interrupt
}

void *schedule(void *current_rsp) {
    static int first_time = 1;

    scheduler.remaining_quantum--;

    if (!scheduler.num_processes || scheduler.remaining_quantum > 0) {
        return current_rsp;
    }

    // Save current process state
    if (scheduler.processes[scheduler.current_pid] != NULL) {
        Process *current_process = (Process *)scheduler.processes[scheduler.current_pid]->data;

        if (!first_time) {
            current_process->stack_pos = current_rsp;
        } else {
            first_time = 0;
        }

        if (current_process->status == RUNNING) {
            current_process->status = READY;
        }

        // Priority aging (TP2_SO style)
        uint8_t new_priority = current_process->priority > 0 ?
                               current_process->priority - 1 :
                               current_process->priority;
        set_priority(scheduler.current_pid, new_priority);
    }

    // Pick next process
    scheduler.current_pid = get_next_pid();
    Process *next_process = (Process *)scheduler.processes[scheduler.current_pid]->data;

    // Set quantum based on priority
    scheduler.remaining_quantum = (NUM_PRIORITIES - 1 - next_process->priority) + 1;

    next_process->status = RUNNING;
    return next_process->stack_pos;
}
