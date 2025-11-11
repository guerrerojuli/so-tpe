 
 
 
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

     
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        args[0] = argv[1];
        printf("Invalid PID: %s\n", args);
        return 1;
    }

     
     
    int64_t result = (int64_t)sys_kill_process(pid, -1);   

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