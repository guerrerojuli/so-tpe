 
 
 
#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int mem_func(int argc, char **argv) {
    uint64_t total = 0, free = 0, used = 0;
    char manager_name[32] = {0};

     
    sys_mem_state((uint64_t)&total, (uint64_t)&free, (uint64_t)&used, (uint64_t)manager_name);

     
    void *args[1];
    args[0] = manager_name;
    printf("%s Memory Manager:\n", args);

     
     
    int total_int = (int)total;
    int used_int = (int)used;
    int free_int = (int)free;

     
    int total_kb = (int)(total / 1024);
    int total_mb = (int)(total_kb / 1024);
    int used_kb = (int)(used / 1024);
    int used_mb = (int)(used_kb / 1024);
    int free_kb = (int)(free / 1024);
    int free_mb = (int)(free_kb / 1024);

     
    void *total_args[3] = {&total_int, &total_kb, &total_mb};
    printf("Total Memory: %d bytes (%d KB, %d MB)\n", total_args);

     
    void *used_args[3] = {&used_int, &used_kb, &used_mb};
    printf("Used Memory:  %d bytes (%d KB, %d MB)\n", used_args);

     
    void *free_args[3] = {&free_int, &free_kb, &free_mb};
    printf("Free Memory:  %d bytes (%d KB, %d MB)\n", free_args);

     
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
    "Display memory state"
};