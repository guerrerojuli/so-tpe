#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "commands.h"

// Forward declaration from test_mm.c
extern uint64_t test_mm(uint64_t argc, char *argv[]);

static int test_mm_func(int argc, char **argv) {
    // Call test_mm with argv
    // argc includes the command name, so we pass argc - 1
    if (argc <= 1) {
        printf("Usage: test-mm <max_memory>\n", NULL);
        printf("  max_memory: maximum memory to use in bytes\n", NULL);
        return -1;
    }

    return test_mm(argc - 1, &argv[1]);
}

command test_mm_cmd = {
    "test-mm",
    test_mm_func,
    "Run memory manager test"
};