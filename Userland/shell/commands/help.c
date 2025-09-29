#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int help_func(void) {
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
    "Shows this help message",
    "HELP(1)                     User Commands                     HELP(1)\n\n"
    "NAME\n"
    "       help - display available commands\n\n"
    "SYNOPSIS\n"
    "       help\n\n"
    "DESCRIPTION\n"
    "       Display a list of all available commands with brief descriptions.\n"
    "       For detailed information about a specific command, use 'man <command>'.\n\n"
}; 