// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// Scheduler implementation - priority-based round-robin with aging

#include <stddef.h>
#include "include/scheduler.h"
#include "include/list.h"
#include "include/memoryManager.h"
#include "include/pipe.h"
#include "include/globals.h"
#include "include/consoleDriver.h"

typedef struct
{
    Node *processes[MAX_PROCESSES];    // Fast PID lookup
    List ready_queues[NUM_PRIORITIES]; // Priority queues
    List blocked_queue;
    uint16_t current_pid;
    uint16_t next_unused_pid;
    uint16_t num_processes;
    int16_t remaining_quantum;      // Changed to int16_t to support larger quantum values
    int16_t initial_quantum;        // Quantum assigned at start of time slice
    uint8_t kill_fg_flag;           // Flag to kill foreground process (Ctrl+C)
    uint16_t foreground_pid;        // PID of the current foreground process (0 = none)
    uint8_t current_priority_level; // Current priority level for round-robin rotation
} Scheduler;

static Scheduler scheduler;

// Calculate base quantum for a given priority
// Priority 4 (highest) -> 64, Priority 3 -> 32, Priority 2 -> 16, Priority 1 -> 8, Priority 0 (lowest) -> 4
static int16_t calculate_base_quantum(uint8_t priority)
{
    return 4 * (1 << priority);
}

// Calculate adjusted quantum based on I/O bound behavior
static int16_t calculate_adjusted_quantum(Process *process)
{
    int16_t base_quantum = calculate_base_quantum(process->priority);

    if (process->is_io_bound && process->quantum_usage_percent > 0)
    {
        // For I/O bound processes, increase quantum inversely to usage
        // If process uses 25% of quantum, multiply by 4 (100/25)
        int16_t adjusted = (base_quantum * 100) / process->quantum_usage_percent;

        // Cap at 4x the base quantum to prevent excessive values
        if (adjusted > base_quantum * 4)
        {
            adjusted = base_quantum * 4;
        }

        return adjusted;
    }

    return base_quantum;
}

// Update I/O bound status based on quantum usage
static void update_io_bound_status(Process *process, uint8_t quantum_used, int16_t total_quantum)
{
    if (total_quantum > 0)
    {
        uint8_t usage_percent = (quantum_used * 100) / total_quantum;

        // Update rolling average (weighted: 75% old, 25% new)
        process->quantum_usage_percent = (process->quantum_usage_percent * 3 + usage_percent) / 4;

        // Mark as I/O bound if consistently uses less than 50% of quantum
        process->is_io_bound = (process->quantum_usage_percent < 50);

        process->last_quantum_used = quantum_used;
    }
}

void scheduler_init()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        scheduler.processes[i] = NULL;
    }

    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        list_init(&scheduler.ready_queues[i]);
    }

    list_init(&scheduler.blocked_queue);
    scheduler.current_pid = 0;
    scheduler.next_unused_pid = 0;
    scheduler.num_processes = 0;
    scheduler.remaining_quantum = 1;
    scheduler.foreground_pid = 0;                          // No foreground process initially
    scheduler.current_priority_level = NUM_PRIORITIES - 1; // Start at highest priority
}

// Find next ready process using round-robin priority rotation
static uint16_t get_next_pid()
{
    // Try all priority levels starting from current_priority_level
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        int lvl = scheduler.current_priority_level;

        if (!list_is_empty(&scheduler.ready_queues[lvl]))
        {
            Node *node = list_get_first(&scheduler.ready_queues[lvl]);
            Process *process = (Process *)node->data;

            // Rotate to next priority level for next scheduling decision (4->3->2->1->0->4)
            scheduler.current_priority_level = (scheduler.current_priority_level - 1 + NUM_PRIORITIES) % NUM_PRIORITIES;

            return process->pid;
        }

        // Move to next priority level
        scheduler.current_priority_level = (scheduler.current_priority_level - 1 + NUM_PRIORITIES) % NUM_PRIORITIES;
    }

    return IDLE_PID;
}

int16_t create_process(MainFunction code, char **args, char *name,
                       uint8_t priority, int16_t fds[3], uint8_t unkillable)
{
    if (scheduler.num_processes >= MAX_PROCESSES)
        return -1;

    if (priority >= NUM_PRIORITIES)
        priority = NUM_PRIORITIES - 1;

    // Allocate PCB
    Process *process = (Process *)mm_alloc(sizeof(Process));
    if (process == NULL)
    {
        return -1;
    }

    if (init_process(process, scheduler.next_unused_pid, scheduler.current_pid,
                     code, args, name, priority, fds, unkillable) != 0)
    {
        mm_free(process);
        return -1;
    }

    // Add to scheduler
    Node *node;
    if (process->pid != IDLE_PID)
    {
        node = list_append(&scheduler.ready_queues[priority], process);
        if (node == NULL)
        {
            free_process(process);
            mm_free(process);
            return -1;
        }
    }
    else
    {
        // IDLE is special - manual node, not in queue
        node = (Node *)mm_alloc(sizeof(Node));
        if (node == NULL)
        {
            free_process(process);
            mm_free(process);
            return -1;
        }
        node->data = process;
        node->prev = node->next = NULL;
    }

    scheduler.processes[process->pid] = node;

    // Find next unused PID
    while (scheduler.processes[scheduler.next_unused_pid] != NULL)
    {
        scheduler.next_unused_pid = (scheduler.next_unused_pid + 1) % MAX_PROCESSES;
    }

    scheduler.num_processes++;
    return process->pid;
}

int8_t set_priority(uint16_t pid, uint8_t new_priority)
{
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL || pid == IDLE_PID)
        return -1;

    if (new_priority >= NUM_PRIORITIES)
        return -1;

    Node *node = scheduler.processes[pid];
    Process *process = (Process *)node->data;

    if (process->status == READY || process->status == RUNNING)
    {
        // Remove from old queue
        list_remove(&scheduler.ready_queues[process->priority], node);

        // Add to new queue
        node = list_append(&scheduler.ready_queues[new_priority], process);
        if (node == NULL)
        {
            scheduler.processes[pid] = NULL;
            scheduler.num_processes--;
            free_process(process);
            mm_free(process);
            return -1;
        }
        scheduler.processes[pid] = node;
    }

    process->priority = new_priority;
    return new_priority;
}

int8_t set_status(uint16_t pid, ProcessStatus new_status)
{
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

    if (new_status == BLOCKED)
    {
        // Calculate quantum usage when blocking
        if (pid == scheduler.current_pid && scheduler.initial_quantum > 0)
        {
            int16_t quantum_used = scheduler.initial_quantum - scheduler.remaining_quantum;
            update_io_bound_status(process, quantum_used, scheduler.initial_quantum);
        }

        // Demote priority - I/O-bound processes move DOWN to lower priorities
        uint8_t old_priority = process->priority;
        if (process->priority > 0)
        {
            process->priority--;
        }

        // Reset quantum consumption counter when blocking (I/O activity)
        process->quantum_consumed_count = 0;

        // Move to blocked queue (remove from old priority queue)
        list_remove(&scheduler.ready_queues[old_priority], node);
        node = list_append(&scheduler.blocked_queue, process);
        if (node == NULL)
        {
            scheduler.processes[pid] = NULL;
            scheduler.num_processes--;
            free_process(process);
            mm_free(process);
            return -1;
        }
        scheduler.processes[pid] = node;
    }
    else if (old_status == BLOCKED && new_status == READY)
    {
        // Move back to ready with max priority (priority boost)
        list_remove(&scheduler.blocked_queue, node);
        process->priority = NUM_PRIORITIES - 1;
        node = list_prepend(&scheduler.ready_queues[process->priority], process);
        if (node == NULL)
        {
            scheduler.processes[pid] = NULL;
            scheduler.num_processes--;
            free_process(process);
            mm_free(process);
            return -1;
        }
        scheduler.processes[pid] = node;
        scheduler.remaining_quantum = 0; // Force reschedule
    }

    return new_status;
}

int32_t kill_process(uint16_t pid, int32_t retval)
{
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL)
        return -1;

    Node *node = scheduler.processes[pid];
    Process *process = (Process *)node->data;

    if (process->status == ZOMBIE || process->unkillable)
        return -1;

    // Remove from queue
    if (process->status == BLOCKED)
    {
        list_remove(&scheduler.blocked_queue, node);
    }
    else
    {
        list_remove(&scheduler.ready_queues[process->priority], node);
    }

    // Cascade cleanup: destroy all zombie children before this process becomes zombie
    while (!list_is_empty(&process->zombie_children))
    {
        Node *zombie_node = list_get_first(&process->zombie_children);
        Process *zombie_child = (Process *)zombie_node->data;
        uint16_t zombie_pid = zombie_child->pid;

        // Save the node from scheduler.processes before we remove from zombie list
        Node *scheduler_node = scheduler.processes[zombie_pid];

        // Remove from parent's zombie list
        list_remove(&process->zombie_children, zombie_node); // This frees zombie_node from parent's list

        // Remove from main process table and free the scheduler's node
        scheduler.processes[zombie_pid] = NULL;
        scheduler.num_processes--;

        // Free zombie child resources
        free_process(zombie_child);
        mm_free(zombie_child); // Free the Process structure

        // Free the node that was in scheduler.processes (this is the same as zombie_node since we updated it)
        // Note: zombie_node was already freed by list_remove, and it's the same as scheduler_node
        // so we don't need to free scheduler_node separately (it's already freed)
    }

    process->status = ZOMBIE;
    process->return_value = retval;

    // Close all pipe file descriptors immediately to send EOF to readers
    // This is critical for pipes - readers need to get EOF when writer dies
    for (int i = 0; i < 3; i++)
    {
        int16_t fd = process->file_descriptors[i];
        if (fd >= BUILT_IN_DESCRIPTORS)
        {
            // It's a pipe - close it now to send EOF
            pipe_close_for_pid(pid, fd);
            process->file_descriptors[i] = -1; // Mark as closed
        }
    }

    // Clear foreground status if this was the foreground process
    if (pid == scheduler.foreground_pid)
    {
        scheduler.foreground_pid = 0;
    }

    // Handle zombie based on parent status
    uint16_t parent_pid = process->parent_pid;
    if (parent_pid < MAX_PROCESSES && scheduler.processes[parent_pid] != NULL)
    {
        Process *parent = (Process *)scheduler.processes[parent_pid]->data;

        // If parent is alive (not a zombie), add to parent's zombie children list
        if (parent->status != ZOMBIE)
        {
            // Add zombie to parent's zombie_children list
            // CRITICAL: list_append returns a new node, we must update scheduler.processes[pid]
            // to avoid use-after-free (the old node was freed by list_remove above)
            Node *zombie_node = list_append(&parent->zombie_children, process);
            if (zombie_node == NULL)
            {
                // Can't add to zombie list - must auto-reap this orphan
                scheduler.processes[pid] = NULL;
                scheduler.num_processes--;
                free_process(process);
                mm_free(process);
                return 0;
            }
            scheduler.processes[pid] = zombie_node;

            // If parent is waiting for this specific child, unblock it
            if (parent->waiting_for_pid == pid && parent->status == BLOCKED)
            {
                set_status(parent_pid, READY);
            }
        }
        else
        {
            // Parent is zombie - auto-reap this orphan
            scheduler.processes[pid] = NULL;
            scheduler.num_processes--;
            free_process(process);
            mm_free(process); // Free the Process structure itself
            // NOTE: Don't free node - it was already freed by list_remove above
        }
    }
    else
    {
        // Parent doesn't exist - auto-reap this orphan
        scheduler.processes[pid] = NULL;
        scheduler.num_processes--;
        free_process(process);
        mm_free(process); // Free the Process structure itself
        // NOTE: Don't free node - it was already freed by list_remove above
    }

    // If current process is dying, force context switch
    if (pid == scheduler.current_pid)
    {
        yield();
    }

    return 0;
}

int32_t kill_current_process(int32_t retval)
{
    return kill_process(scheduler.current_pid, retval);
}

// Called by keyboard driver when Ctrl+C is pressed
void kill_foreground_process(void)
{
    scheduler.kill_fg_flag = 1;
}

uint16_t get_pid()
{
    return scheduler.current_pid;
}

uint16_t get_foreground_pid()
{
    return scheduler.foreground_pid;
}

void yield()
{
    // Track quantum usage when process voluntarily yields
    if (scheduler.processes[scheduler.current_pid] != NULL && scheduler.initial_quantum > 0)
    {
        Process *current_process = (Process *)scheduler.processes[scheduler.current_pid]->data;
        int16_t quantum_used = scheduler.initial_quantum - scheduler.remaining_quantum;
        update_io_bound_status(current_process, quantum_used, scheduler.initial_quantum);

        // Demote priority - I/O-bound processes move DOWN to lower priorities
        if (current_process->priority > 0)
        {
            set_priority(scheduler.current_pid, current_process->priority - 1);
        }

        // Reset quantum consumption counter on voluntary yield (indicates I/O activity)
        current_process->quantum_consumed_count = 0;
    }

    scheduler.remaining_quantum = 0;
    __asm__ volatile("int $0x20"); // Use timer interrupt to ensure EOI is sent
}

void *schedule(void *current_rsp)
{
    static int first_time = 1;

    // Check if we need to kill foreground process (Ctrl+C)
    if (scheduler.kill_fg_flag)
    {
        scheduler.kill_fg_flag = 0; // Clear flag

        // TP2_SO approach: Kill current process if it has STDIN
        // This is more reliable than tracking a separate foreground_pid
        if (scheduler.current_pid != IDLE_PID &&
            scheduler.processes[scheduler.current_pid] != NULL)
        {
            Process *current = (Process *)scheduler.processes[scheduler.current_pid]->data;
            // Check if this process has access to keyboard (STDIN)
            if (current->file_descriptors[0] == STDIN)
            {
                kill_current_process(-1); // Kill with signal -1 (interrupted)
            }
        }
    }

    scheduler.remaining_quantum--;

    if (!scheduler.num_processes || scheduler.remaining_quantum > 0)
    {
        return current_rsp;
    }

    // Save current process state
    if (scheduler.processes[scheduler.current_pid] != NULL)
    {
        Process *current_process = (Process *)scheduler.processes[scheduler.current_pid]->data;

        if (!first_time)
        {
            current_process->stack_pos = current_rsp;
        }
        else
        {
            first_time = 0;
        }

        if (current_process->status == RUNNING)
        {
            current_process->status = READY;

            uint8_t rotated = 0; // Track if we already rotated via set_priority

            // Process consumed entire quantum
            if (scheduler.remaining_quantum == 0 && scheduler.initial_quantum > 0)
            {
                // Update I/O bound status (used 100% of quantum)
                update_io_bound_status(current_process, scheduler.initial_quantum, scheduler.initial_quantum);

                // Increment quantum consumption counter
                current_process->quantum_consumed_count++;

                // Delayed priority promotion - CPU-bound processes move UP to higher priorities
                if (current_process->quantum_consumed_count >= AGING_THRESHOLD && current_process->priority < NUM_PRIORITIES - 1)
                {
                    uint8_t new_priority = current_process->priority + 1;
                    set_priority(scheduler.current_pid, new_priority);
                    rotated = 1; // set_priority rotates the process

                    // Reset counter after promotion
                    current_process->quantum_consumed_count = 0;
                }
            }

            // Always rotate to end of queue for round-robin (if not already done via aging)
            if (!rotated)
            {
                set_priority(scheduler.current_pid, current_process->priority);
            }
        }
    }

    // Pick next process
    scheduler.current_pid = get_next_pid();
    Process *next_process = (Process *)scheduler.processes[scheduler.current_pid]->data;

    // Set quantum with dynamic adjustment for I/O bound processes
    scheduler.initial_quantum = calculate_adjusted_quantum(next_process);
    scheduler.remaining_quantum = scheduler.initial_quantum;

    next_process->status = RUNNING;
    return next_process->stack_pos;
}

int32_t waitpid(uint16_t pid)
{
    // Check if child process exists
    if (pid >= MAX_PROCESSES || scheduler.processes[pid] == NULL)
        return -1;

    Node *child_node = scheduler.processes[pid];
    Process *child_process = (Process *)child_node->data;

    // Check if calling process is the parent
    if (child_process->parent_pid != scheduler.current_pid)
        return -1;

    // Get parent process
    Process *parent = (Process *)scheduler.processes[scheduler.current_pid]->data;
    parent->waiting_for_pid = pid;

    // Set child as foreground process (parent is waiting for it)
    scheduler.foreground_pid = pid;

    // If child hasn't terminated yet, block parent until it does
    if (child_process->status != ZOMBIE)
    {
        set_status(parent->pid, BLOCKED);
        yield();
    }

    // Child is zombie, collect return value
    int32_t retval = child_process->return_value;

    // Remove zombie from parent's zombie_children list
    Node *zombie_node = list_get_first(&parent->zombie_children);
    while (zombie_node != NULL)
    {
        Process *zombie = (Process *)zombie_node->data;
        if (zombie->pid == pid)
        {
            list_remove(&parent->zombie_children, zombie_node); // This already frees zombie_node
            break;
        }
        zombie_node = zombie_node->next;
    }

    // Clean up zombie process from main table
    scheduler.processes[pid] = NULL;
    scheduler.num_processes--;
    free_process(child_process);
    mm_free(child_process); // Free the Process structure itself
    // NOTE: Don't free child_node - it's the same as zombie_node which was already freed by list_remove above

    // Clear waiting state and foreground status
    parent->waiting_for_pid = 0;
    scheduler.foreground_pid = 0; // No more foreground process

    return retval;
}

// Get file descriptor for current process
int16_t get_process_fd(uint8_t fd_index)
{
    if (fd_index >= 3) // Only stdin(0), stdout(1), stderr(2)
        return -1;

    if (scheduler.processes[scheduler.current_pid] == NULL)
        return -1;

    Process *current = (Process *)scheduler.processes[scheduler.current_pid]->data;
    return current->file_descriptors[fd_index];
}

int32_t get_process_info(ProcessInfo *info_array, uint32_t max_count)
{
    if (!info_array || max_count == 0)
        return -1;

    uint32_t count = 0;
    for (int i = 0; i < MAX_PROCESSES && count < max_count; i++)
    {
        if (scheduler.processes[i] != NULL)
        {
            Process *process = (Process *)scheduler.processes[i]->data;

            // Fill process info
            info_array[count].pid = process->pid;
            info_array[count].parent_pid = process->parent_pid;

            // Copy process name safely
            int j;
            for (j = 0; j < 63 && process->name[j] != '\0'; j++)
            {
                info_array[count].name[j] = process->name[j];
            }
            info_array[count].name[j] = '\0';

            info_array[count].priority = process->priority;
            info_array[count].status = process->status;
            info_array[count].stack_base = process->stack_base;
            info_array[count].stack_pos = process->stack_pos;

            // Determine if foreground: check if this is the foreground process
            // Only the process being waited on by waitpid() is foreground
            info_array[count].is_foreground = (process->pid == scheduler.foreground_pid) ? 1 : 0;

            count++;
        }
    }

    return count;
}