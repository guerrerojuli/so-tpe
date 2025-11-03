#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "commands.h"

// Forward declaration from test_sync.c
extern uint64_t test_sync(uint64_t argc, char *argv[]);

static int test_sync_func(int argc, char **argv) {
    // Call test_sync with argv
    // argc includes the command name, so we pass argc - 1
    if (argc <= 1) {
        printf("Usage: test-sync <n> <use_sem>\n", NULL);
        printf("  n: number of iterations\n", NULL);
        printf("  use_sem: 0=no synchronization, 1=use semaphores\n", NULL);
        return -1;
    }
    
    return test_sync(argc - 1, &argv[1]);
}

command test_sync_cmd = {
    "test-sync", 
    test_sync_func, 
    "Run synchronization test with semaphores",
    "TEST-SYNC(1)                User Commands                TEST-SYNC(1)\n\n"
    "NAME\n"
    "       test-sync - test process synchronization with semaphores\n\n"
    "SYNOPSIS\n"
    "       test-sync <n> <use_sem>\n\n"
    "DESCRIPTION\n"
    "       Tests process synchronization using semaphores. Creates multiple\n"
    "       processes that increment and decrement a shared variable.\n\n"
    "PARAMETERS\n"
    "       n          Number of iterations per process\n"
    "       use_sem    0 = no synchronization (demonstrates race condition)\n"
    "                  1 = use semaphores (proper synchronization)\n\n"
    "EXAMPLES\n"
    "       test-sync 10000 0    # Show race condition\n"
    "       test-sync 10000 1    # Show proper synchronization\n\n"
}; 

