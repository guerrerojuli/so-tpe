// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
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