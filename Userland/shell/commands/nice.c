// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../commands/commands.h"

int nice_main(int argc, char **argv) {
    void *args[] = {(void*)(uint64_t)0, (void*)(uint64_t)0};

    // Validate argument count
    if (argc != 3) {
        printf("Usage: nice <pid> <priority>\n", args);
        printf("Priority must be between 0 (lowest) and 4 (highest)\n", args);
        return 1;
    }

    // Convert string PID to integer
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        args[0] = argv[1];
        printf("Invalid PID: %s\n", args);
        return 1;
    }

    // Convert string priority to integer
    int priority = atoi(argv[2]);

    // Check if the conversion was valid (atoi returns 0 for non-numeric strings)
    // but 0 is also a valid priority, so we need to check the actual string
    if (priority == 0 && argv[2][0] != '0') {
        args[0] = argv[2];
        printf("Invalid priority: %s (must be a number)\n", args);
        return 1;
    }

    // Validate priority range (0-4)
    if (priority < 0 || priority > 4) {
        args[0] = argv[2];
        printf("Invalid priority: %s (must be between 0 and 4)\n", args);
        return 1;
    }

    // Call set_priority syscall
    int result = sys_set_priority(pid, priority);

    if (result < 0) {
        args[0] = (void*)&pid;
        printf("Failed to change priority for process %d (process not found or is idle process)\n", args);
        return 1;
    }

    args[0] = (void*)&pid;
    args[1] = (void*)&priority;
    printf("Process %d priority changed to %d\n", args);
    return 0;
}

command nice_cmd = {
    "nice",
    nice_main,
    "Change process priority"
};