// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// Process management implementation

#include "include/process.h"
#include "include/memoryManager.h"
#include "include/lib.h"
#include "include/pipe.h"
#include "include/globals.h"
#include "include/list.h"
#include <stddef.h>

extern void *_initialize_stack_frame(void *wrapper, void *code, void *stack_top, void *args);
extern int32_t kill_current_process(int32_t retval);

// Wrapper that handles process exit
void process_wrapper(MainFunction code, char **args) {
    int argc = 0;
    if (args) {
        while (args[argc] != NULL) argc++;
    }
    int retval = code(argc, args);
    kill_current_process(retval);
}

// Deep copy arguments into contiguous memory
static char **allocate_arguments(char **args) {
    if (!args) return NULL;

    // Count arguments and calculate total size
    int argc = 0;
    int total_size = 0;
    while (args[argc] != NULL) {
        total_size += strlen(args[argc]) + 1;
        argc++;
    }

    // Allocate: pointers array + strings
    total_size += sizeof(char *) * (argc + 1);
    char **new_args = (char **)mm_alloc(total_size);
    if (new_args == NULL) {
        return NULL;
    }

    // Copy strings into contiguous memory
    char *str_area = (char *)new_args + sizeof(char *) * (argc + 1);
    for (int i = 0; i < argc; i++) {
        new_args[i] = str_area;
        strcpy(str_area, args[i]);
        str_area += strlen(args[i]) + 1;
    }
    new_args[argc] = NULL;

    return new_args;
}

int8_t init_process(Process *process, uint16_t pid, uint16_t parent_pid,
                    MainFunction code, char **args, char *name,
                    uint8_t priority, int16_t fds[3], uint8_t unkillable) {
    process->pid = pid;
    process->parent_pid = parent_pid;
    process->priority = priority;
    process->status = READY;
    process->unkillable = unkillable;
    process->return_value = 0;

    // Initialize quantum tracking fields
    process->quantum_consumed_count = 0;
    process->last_quantum_used = 0;
    process->quantum_usage_percent = 100;  // Assume CPU-bound initially
    process->is_io_bound = 0;              // Not I/O bound initially

    // Initialize wait fields
    process->waiting_for_pid = 0;
    list_init(&process->zombie_children);

    // Allocate stack (4KB)
    process->stack_base = mm_alloc(4096);
    if (process->stack_base == NULL) {
        return -1;
    }

    // Copy name
    process->name = (char *)mm_alloc(strlen(name) + 1);
    if (process->name == NULL) {
        mm_free(process->stack_base);
        return -1;
    }
    strcpy(process->name, name);

    // Copy arguments
    process->argv = allocate_arguments(args);
    if (args != NULL && process->argv == NULL) {
        mm_free(process->name);
        mm_free(process->stack_base);
        return -1;
    }

    // Setup initial stack frame
    void *stack_top = (void *)((uint64_t)process->stack_base + PROCESS_STACK_SIZE);
    process->stack_pos = _initialize_stack_frame(process_wrapper, code, stack_top, process->argv);

    // Copy file descriptors
    process->file_descriptors[0] = fds[0];
    process->file_descriptors[1] = fds[1];
    process->file_descriptors[2] = fds[2];

    // Auto-open pipes when process is created with pipe FDs
    for (int i = 0; i < 3; i++) {
        if (fds[i] >= BUILT_IN_DESCRIPTORS) {
            // This is a pipe - open it for this process
            uint8_t mode = (i == 0) ? READ : WRITE;  // stdin is READ, stdout/stderr are WRITE
            pipe_open_for_pid(process->pid, fds[i], mode);
        }
    }

    return 0;
}

void free_process(Process *process) {
    // Close all pipe file descriptors
    for (int i = 0; i < 3; i++) {
        int16_t fd = process->file_descriptors[i];
        if (fd >= BUILT_IN_DESCRIPTORS) {
            pipe_close_for_pid(process->pid, fd);
        }
    }

    mm_free(process->stack_base);
    if (process->name)
        mm_free(process->name);
    if (process->argv)
        mm_free(process->argv);
}
