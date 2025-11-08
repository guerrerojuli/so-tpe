#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "commands.h"

// Forward declaration from test_processes.c
extern int64_t test_processes(uint64_t argc, char *argv[]);

static int test_processes_func(int argc, char **argv) {
    // Call test_processes with argv
    // argc includes the command name, so we pass argc - 1
    if (argc <= 1) {
        printf("Usage: test-processes <max_processes>\n", NULL);
        printf("  max_processes: maximum number of processes to create\n", NULL);
        return -1;
    }

    return test_processes(argc - 1, &argv[1]);
}

command test_processes_cmd = {
    "test-processes",
    test_processes_func,
    "Run process management test",
    "TEST-PROCESSES(1)           User Commands           TEST-PROCESSES(1)\n\n"
    "NAME\n"
    "       test-processes - test process creation, blocking, and killing\n\n"
    "SYNOPSIS\n"
    "       test-processes <max_processes>\n\n"
    "DESCRIPTION\n"
    "       Tests process management by randomly creating, blocking,\n"
    "       unblocking, and killing processes in an infinite loop.\n"
    "       This test verifies:\n"
    "       - Process creation (my_create_process)\n"
    "       - Process killing (my_kill)\n"
    "       - Process blocking (my_block)\n"
    "       - Process unblocking (my_unblock)\n\n"
    "PARAMETERS\n"
    "       max_processes  Maximum number of processes to create\n"
    "                      in each iteration\n\n"
    "EXAMPLES\n"
    "       test-processes 5     # Create up to 5 test processes\n"
    "       test-processes 10    # Create up to 10 test processes\n\n"
    "NOTES\n"
    "       - This test runs indefinitely; use Ctrl+C to stop\n"
    "       - Each iteration creates max_processes, then randomly\n"
    "         blocks/unblocks/kills them until all are killed\n"
    "       - Useful for stress-testing the scheduler and process manager\n\n"
};
