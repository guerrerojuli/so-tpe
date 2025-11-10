// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../commands/commands.h"

#define MAX_PROCESSES 20

int block_main(int argc, char **argv) {
    void *args[2] = {(void*)(uint64_t)0, (void*)(uint64_t)0};

    // Validate argument count
    if (argc != 2) {
        printf("Usage: block <pid>\n", args);
        return 1;
    }

    // Convert string PID to integer
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        args[0] = argv[1];
        printf("Invalid PID: %s\n", args);
        return 1;
    }

    // Get current process information to determine its state
    ProcessInfo info[MAX_PROCESSES];
    int count = sys_get_process_info(info, MAX_PROCESSES);

    if (count < 0) {
        printf("Failed to get process information\n", args);
        return 1;
    }

    // Find the target process in the info array
    ProcessStatus current_status = -1;
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (info[i].pid == pid) {
            current_status = info[i].status;
            found = 1;
            break;
        }
    }

    if (!found) {
        args[0] = (void*)&pid;
        printf("Process %d not found\n", args);
        return 1;
    }

    // Check if process is ZOMBIE (cannot be blocked/unblocked)
    if (current_status == ZOMBIE) {
        args[0] = (void*)&pid;
        printf("Cannot block/unblock zombie process %d\n", args);
        return 1;
    }

    // Toggle based on current state
    int result;
    const char *action;

    if (current_status == BLOCKED) {
        // Process is blocked, unblock it
        result = sys_unblock(pid);
        action = "unblocked";
    } else {
        // Process is ready or running, block it
        result = sys_block(pid);
        action = "blocked";
    }

    if (result < 0) {
        args[0] = (void*)&pid;
        printf("Failed to toggle block state for process %d (is it the idle process?)\n", args);
        return 1;
    }

    args[0] = (void*)&pid;
    args[1] = (void*)action;
    printf("Process %d %s\n", args);

    return 0;
}

command block_cmd = {
    "block",
    block_main,
    "Toggle process block state"
};