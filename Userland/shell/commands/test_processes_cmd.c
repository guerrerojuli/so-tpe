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
    "Run process management test"
};
