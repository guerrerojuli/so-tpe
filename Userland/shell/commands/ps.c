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

// Convert uint64_t to hexadecimal string
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

    // Print header
    printf("PID  | PPID | NAME             | PRIORITY | STATUS   | FG | STACK_BASE         | STACK_POS\n", NULL);
    printf("-----|------|------------------|----------|----------|----|--------------------|------------------\n", NULL);

    // Print each process
    for (int i = 0; i < count; i++)
    {
        void *args[1];
        char hex_buffer[20];
        
        // Print PID
        int pid = (int)processes[i].pid;
        args[0] = &pid;
        printf("%d", args);
        
        // Add spacing (adjust based on PID size)
        if (pid < 10) printf("    ", NULL);
        else if (pid < 100) printf("   ", NULL);
        else if (pid < 1000) printf("  ", NULL);
        else printf(" ", NULL);
        
        printf("| ", NULL);

        // Print PPID
        int ppid = (int)processes[i].parent_pid;
        args[0] = &ppid;
        printf("%d", args);
        
        // Add spacing
        if (ppid < 10) printf("    ", NULL);
        else if (ppid < 100) printf("   ", NULL);
        else if (ppid < 1000) printf("  ", NULL);
        else printf(" ", NULL);
        
        printf("| ", NULL);

        // Print Name (18 chars width, left-aligned)
        args[0] = processes[i].name;
        printf("%s", args);
        
        // Add padding for name alignment
        int name_len = strlen(processes[i].name);
        int padding = 17 - name_len;
        if (padding < 1)
            padding = 1;
        for (int j = 0; j < padding; j++)
            printf(" ", NULL);
        
        printf("| ", NULL);

        // Print Priority
        int priority = (int)processes[i].priority;
        args[0] = &priority;
        printf("%d", args);
        printf("        ", NULL);
        
        printf("| ", NULL);

        // Print Status (9 chars width)
        const char *status_str = status_to_string(processes[i].status);
        args[0] = (void *)status_str;
        printf("%s", args);
        
        // Add padding for status
        int status_len = strlen(status_str);
        padding = 9 - status_len;
        if (padding < 1)
            padding = 1;
        for (int j = 0; j < padding; j++)
            printf(" ", NULL);
        
        printf("| ", NULL);

        // Print Foreground
        args[0] = processes[i].is_foreground ? (void *)"Y" : (void *)"N";
        printf("%s", args);
        printf("  ", NULL);
        
        printf("| ", NULL);

        // Print Stack Base
        uint64_to_hex((uint64_t)processes[i].stack_base, hex_buffer);
        args[0] = hex_buffer;
        printf("%s", args);
        printf(" ", NULL);
        
        printf("| ", NULL);

        // Print Stack Pos
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
    "List all processes with their properties",
    "PS(1)                       User Commands                       PS(1)\n\n"
    "NAME\n"
    "       ps - report process status\n\n"
    "SYNOPSIS\n"
    "       ps\n\n"
    "DESCRIPTION\n"
    "       Display information about all active processes in the system.\n"
    "       Shows process ID (PID), parent process ID (PPID), process name,\n"
    "       priority level, current status, foreground/background flag,\n"
    "       stack base pointer, and current stack pointer.\n\n"
    "FIELDS\n"
    "       PID      Process ID\n"
    "       PPID     Parent Process ID\n"
    "       NAME     Process name\n"
    "       PRIORITY Process priority (0-4, higher is more priority)\n"
    "       STATUS   Current process state (READY/RUNNING/BLOCKED/ZOMBIE)\n"
    "       FG       Foreground flag (Y/N)\n"
    "       STACK_BASE   Base address of process stack\n"
    "       STACK_POS    Current stack pointer position\n\n"};

