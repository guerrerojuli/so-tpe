 
 
 
#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int help_func(int argc, char **argv) {
    printf("Available commands:\n", NULL);
    for (int i = 0; all_commands[i] != NULL; i++) {
        void *args[2] = {all_commands[i]->name, all_commands[i]->description};
        printf("  %s - %s\n", args);
    }
    return 0;
}

command help_cmd = {
    "help",
    help_func,
    "Shows this help message"
}; 