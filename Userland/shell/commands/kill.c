// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../commands/commands.h"

int kill_main(int argc, char **argv) {
    void *args[] = {(void*)(uint64_t)0};

    if (argc != 2) {
        printf("Usage: kill <pid>\n", args);
        return 1;
    }

    // Convert string PID to integer
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        args[0] = argv[1];
        printf("Invalid PID: %s\n", args);
        return 1;
    }

    // Call kill_process syscall
    // Cast to int64_t first to properly handle sign-extended error codes
    int64_t result = (int64_t)sys_kill_process(pid, -1);  // -1 as signal (killed by user)

    if (result < 0) {
        args[0] = (void*)&pid;
        printf("Failed to kill process %d (process not found or unkillable)\n", args);
        return 1;
    }

    args[0] = (void*)&pid;
    printf("Process %d killed\n", args);
    return 0;
}

command kill_cmd = {
    "kill",
    kill_main,
    "Kill a process by PID"
};