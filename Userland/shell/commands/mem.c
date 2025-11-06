#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int mem_func(int argc, char **argv) {
    uint64_t total = 0, free = 0, used = 0;
    char manager_name[32] = {0};

    // Call the syscall to get memory state
    sys_mem_state((uint64_t)&total, (uint64_t)&free, (uint64_t)&used, (uint64_t)manager_name);

    // Print memory manager name
    void *args[1];
    args[0] = manager_name;
    printf("%s Memory Manager:\n", args);

    // Print memory statistics
    // Convert to int for printf (which expects int* for %d)
    int total_int = (int)total;
    int used_int = (int)used;
    int free_int = (int)free;

    // Convert bytes to KB and MB for better readability
    int total_kb = (int)(total / 1024);
    int total_mb = (int)(total_kb / 1024);
    int used_kb = (int)(used / 1024);
    int used_mb = (int)(used_kb / 1024);
    int free_kb = (int)(free / 1024);
    int free_mb = (int)(free_kb / 1024);

    // Print total memory
    void *total_args[3] = {&total_int, &total_kb, &total_mb};
    printf("Total Memory: %d bytes (%d KB, %d MB)\n", total_args);

    // Print used memory
    void *used_args[3] = {&used_int, &used_kb, &used_mb};
    printf("Used Memory:  %d bytes (%d KB, %d MB)\n", used_args);

    // Print free memory
    void *free_args[3] = {&free_int, &free_kb, &free_mb};
    printf("Free Memory:  %d bytes (%d KB, %d MB)\n", free_args);

    // For buddy system, also show page information if available
    #ifdef BUDDY
    int total_pages = (int)(total / 4096);
    int used_pages = (int)(used / 4096);
    int free_pages = (int)(free / 4096);

    void *page_args[3] = {&total_pages, &used_pages, &free_pages};
    printf("\nPage Statistics:\n", NULL);
    printf("Total Pages: %d | Used Pages: %d | Free Pages: %d\n", page_args);
    #endif

    return 0;
}

command mem_cmd = {
    "mem",
    mem_func,
    "Display memory state",
    "Usage: mem\n"
    "Displays the current state of memory management including:\n"
    "  - Memory manager type (First-Fit or Buddy System)\n"
    "  - Total memory available\n"
    "  - Used memory\n"
    "  - Free memory\n"
    "All values are shown in bytes, KB, and MB for convenience.\n"
};