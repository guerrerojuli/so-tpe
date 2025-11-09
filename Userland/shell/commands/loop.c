#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int loop_func(int argc, char **argv) {
    // Require exactly 2 arguments (command name + seconds)
    if (argc != 2) {
        printf("Usage: loop <seconds>\n", NULL);
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

    int64_t pid = sys_get_pid();
    int loop_count = 0;

    while (1) {
        void *args[2] = {&pid, &loop_count};
        printf("[PID %d] Hello from loop! (iteration %d)\n", args);
        loop_count++;

        // Use proper sleep syscall for exact timing
        sleep(seconds);
    }

    return 0;
}

command loop_cmd = {
    "loop",
    loop_func,
    "Print PID every N seconds"
};