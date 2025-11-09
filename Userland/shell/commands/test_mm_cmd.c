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
    "Run memory manager test",
    "TEST-MM(1)                  User Commands                  TEST-MM(1)\n\n"
    "NAME\n"
    "       test-mm - test memory allocation and deallocation\n\n"
    "SYNOPSIS\n"
    "       test-mm <max_memory>\n\n"
    "DESCRIPTION\n"
    "       Tests memory management by allocating and freeing blocks\n"
    "       of random sizes in an infinite loop. This test verifies:\n"
    "       - Memory allocation (malloc)\n"
    "       - Memory deallocation (free)\n"
    "       - Block integrity (no overlapping allocations)\n"
    "       - Memory consistency (data preservation)\n\n"
    "PARAMETERS\n"
    "       max_memory     Maximum amount of memory to use in bytes\n"
    "                      for testing allocations\n\n"
    "EXAMPLES\n"
    "       test-mm 1048576      # Test with 1MB of memory\n"
    "       test-mm 10485760     # Test with 10MB of memory\n"
    "       test-mm 104857600    # Test with 100MB of memory\n\n"
    "NOTES\n"
    "       - This test runs indefinitely; use Ctrl+C to stop\n"
    "       - Each iteration allocates random-sized blocks up to\n"
    "         max_memory, fills them with data, verifies integrity,\n"
    "         then frees all blocks\n"
    "       - Tests both BUDDY and FIRSTFIT memory managers depending\n"
    "         on kernel compilation\n\n"
};