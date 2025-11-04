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
    int result = sys_kill_process(pid, -1);  // -1 as signal (killed by user)

    if (result < 0) {
        args[0] = (void*)(uint64_t)pid;
        printf("Failed to kill process %d (process not found or unkillable)\n", args);
        return 1;
    }

    args[0] = (void*)(uint64_t)pid;
    printf("Process %d killed\n", args);
    return 0;
}

command kill_cmd = {
    "kill",
    kill_main,
    "Kill a process by PID",
    "Usage: kill <pid>\n"
    "Terminates the process with the specified PID.\n"
    "Example: kill 5\n"
};