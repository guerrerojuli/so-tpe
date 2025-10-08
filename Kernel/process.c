// Process management implementation

#include "include/process.h"
#include "include/memoryManager.h"
#include "include/lib.h"
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

void init_process(Process *process, uint16_t pid, uint16_t parent_pid,
                  MainFunction code, char **args, char *name,
                  uint8_t priority, int16_t fds[3], uint8_t unkillable) {
    process->pid = pid;
    process->parent_pid = parent_pid;
    process->priority = priority;
    process->status = READY;
    process->unkillable = unkillable;
    process->return_value = 0;

    // Allocate stack (4KB)
    process->stack_base = mm_alloc(4096);

    // Copy name
    process->name = (char *)mm_alloc(strlen(name) + 1);
    strcpy(process->name, name);

    // Copy arguments
    process->argv = allocate_arguments(args);

    // Setup initial stack frame (will be completed in Phase 4)
    void *stack_top = (void *)((uint64_t)process->stack_base + PROCESS_STACK_SIZE);
    process->stack_pos = _initialize_stack_frame(process_wrapper, code, stack_top, process->argv);

    // Copy file descriptors
    process->file_descriptors[0] = fds[0];
    process->file_descriptors[1] = fds[1];
    process->file_descriptors[2] = fds[2];
}

void free_process(Process *process) {
    mm_free(process->stack_base);
    if (process->name)
        mm_free(process->name);
    if (process->argv)
        mm_free(process->argv);
}
