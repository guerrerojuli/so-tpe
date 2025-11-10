// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "commands.h"

// Forward declaration from test_sync.c
extern uint64_t test_sync(uint64_t argc, char *argv[]);

static int test_no_synchro_func(int argc, char **argv) {
    // Call test_sync with use_sem=0 (without synchronization)
    if (argc != 2) {
        printf("Usage: test-no-synchro <n>\n", NULL);
        printf("  n: number of iterations\n", NULL);
        return -1;
    }

    // Create argv array with n and use_sem=0
    char *sync_argv[] = {argv[1], "0"};
    return test_sync(2, sync_argv);
}

command test_no_synchro_cmd = {
    "test-no-synchro",
    test_no_synchro_func,
    "Run synchronization test WITHOUT semaphores"
};
