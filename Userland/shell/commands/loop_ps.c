#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"
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

static void print_ps_output(void)
{
    ProcessInfo processes[MAX_PROCESS_COUNT];

    int64_t count = sys_get_process_info(processes, MAX_PROCESS_COUNT);

    if (count < 0)
    {
        printf("Error: Failed to retrieve process information\n", NULL);
        return;
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
    printf("Total processes: %d\n\n", total_args);
}

static int loop_ps_func(int argc, char **argv) {
    // Require exactly 2 arguments (command name + seconds)
    if (argc != 2) {
        printf("Usage: loop_ps <seconds>\n", NULL);
        return -1;
    }

    // Parse the seconds argument
    int seconds = 0;
    char *str = argv[1];
    while (*str >= '0' && *str <= '9') {
        seconds = seconds * 10 + (*str - '0');
        str++;
    }

    // Check if entire string was consumed (all numeric)
    if (*str != '\0') {
        void *args[1] = {argv[1]};
        printf("Invalid argument: '%s' is not a valid number\n", args);
        return -1;
    }

    // Check if seconds is valid (greater than 0)
    if (seconds <= 0) {
        void *args[1] = {argv[1]};
        printf("Invalid seconds value: '%s' must be greater than 0\n", args);
        return -1;
    }

    int loop_count = 0;

    while (1) {
        // Print separator
        printf("=== PS OUTPUT #%d ===\n", (void **)&loop_count);

        // Print ps output
        print_ps_output();

        loop_count++;

        // Use proper sleep syscall for exact timing
        sleep(seconds);
    }

    return 0;
}

command loop_ps_cmd = {
    "loop_ps",
    loop_ps_func,
    "Print process list every N seconds",
    "Usage: loop_ps <seconds>\n"
    "Prints the process list (ps) every N seconds.\n"
    "Can be run in background with: loop_ps 5 &\n"
};
