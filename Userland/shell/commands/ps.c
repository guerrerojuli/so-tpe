// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:576
 
 
#include "stdio.h"
#include "stddef.h"
#include "commands.h"
#include "unistd.h"
#include "string.h"

#define MAX_PROCESS_COUNT 20

static const char *status_to_string(ProcessStatus status)
{
    switch (status)
    {
    case READY:
        return "READY";
    case RUNNING:
        return "RUNNING";
    case BLOCKED:
        return "BLOCKED";
    case ZOMBIE:
        return "ZOMBIE";
    default:
        return "UNKNOWN";
    }
}

 
static void uint64_to_hex(uint64_t value, char *buffer)
{
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    
    int i;
    for (i = 0; i < 16; i++)
    {
        buffer[2 + i] = hex_chars[(value >> (60 - i * 4)) & 0xF];
    }
    buffer[18] = '\0';
}

static int ps_func(int argc, char **argv)
{
    ProcessInfo processes[MAX_PROCESS_COUNT];

    int64_t count = sys_get_process_info(processes, MAX_PROCESS_COUNT);

    if (count < 0)
    {
        printf("Error: Failed to retrieve process information\n", NULL);
        return -1;
    }

     
    printf("PID  | PPID | NAME             | PRIORITY | STATUS   | FG | STACK_BASE         | STACK_POS\n", NULL);
    printf("-----|------|------------------|----------|----------|----|--------------------|------------------\n", NULL);

     
    for (int i = 0; i < count; i++)
    {
        void *args[1];
        char hex_buffer[20];
        
         
        int pid = (int)processes[i].pid;
        args[0] = &pid;
        printf("%d", args);
        
         
        if (pid < 10) printf("    ", NULL);
        else if (pid < 100) printf("   ", NULL);
        else if (pid < 1000) printf("  ", NULL);
        else printf(" ", NULL);
        
        printf("| ", NULL);

         
        int ppid = (int)processes[i].parent_pid;
        args[0] = &ppid;
        printf("%d", args);
        
         
        if (ppid < 10) printf("    ", NULL);
        else if (ppid < 100) printf("   ", NULL);
        else if (ppid < 1000) printf("  ", NULL);
        else printf(" ", NULL);
        
        printf("| ", NULL);

         
        args[0] = processes[i].name;
        printf("%s", args);
        
         
        int name_len = strlen(processes[i].name);
        int padding = 17 - name_len;
        if (padding < 1)
            padding = 1;
        for (int j = 0; j < padding; j++)
            printf(" ", NULL);
        
        printf("| ", NULL);

         
        int priority = (int)processes[i].priority;
        args[0] = &priority;
        printf("%d", args);
        printf("        ", NULL);
        
        printf("| ", NULL);

         
        const char *status_str = status_to_string(processes[i].status);
        args[0] = (void *)status_str;
        printf("%s", args);
        
         
        int status_len = strlen(status_str);
        padding = 9 - status_len;
        if (padding < 1)
            padding = 1;
        for (int j = 0; j < padding; j++)
            printf(" ", NULL);
        
        printf("| ", NULL);

         
        args[0] = processes[i].is_foreground ? (void *)"Y" : (void *)"N";
        printf("%s", args);
        printf("  ", NULL);
        
        printf("| ", NULL);

         
        uint64_to_hex((uint64_t)processes[i].stack_base, hex_buffer);
        args[0] = hex_buffer;
        printf("%s", args);
        printf(" ", NULL);
        
        printf("| ", NULL);

         
        uint64_to_hex((uint64_t)processes[i].stack_pos, hex_buffer);
        args[0] = hex_buffer;
        printf("%s", args);
        printf("\n", NULL);
    }

    printf("\n", NULL);
    int total_count = (int)count;
    void *total_args[1];
    total_args[0] = &total_count;
    printf("Total processes: %d\n", total_args);

    return 0;
}

command ps_cmd = {
    "ps",
    ps_func,
    "List all processes with their properties"
};

