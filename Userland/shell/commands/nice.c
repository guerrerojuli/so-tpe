// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 
 
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../commands/commands.h"

int nice_main(int argc, char **argv) {
    void *args[] = {(void*)(uint64_t)0, (void*)(uint64_t)0};

     
    if (argc != 3) {
        printf("Usage: nice <pid> <priority>\n", args);
        printf("Priority must be between 0 (lowest) and 4 (highest)\n", args);
        return 1;
    }

     
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        args[0] = argv[1];
        printf("Invalid PID: %s\n", args);
        return 1;
    }

     
    int priority = atoi(argv[2]);

     
     
    if (priority == 0 && argv[2][0] != '0') {
        args[0] = argv[2];
        printf("Invalid priority: %s (must be a number)\n", args);
        return 1;
    }

     
    if (priority < 0 || priority > 4) {
        args[0] = argv[2];
        printf("Invalid priority: %s (must be between 0 and 4)\n", args);
        return 1;
    }

     
     
    int64_t result = (int64_t)sys_set_priority(pid, priority);

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