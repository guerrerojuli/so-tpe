#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int echo_func(void) {
    for (int i = 1; i < arg_count; i++) {
        printf("%s", (void**)&current_args[i]);
        if (i < arg_count - 1) {
            printf(" ", NULL);
        }
    }
    printf("\n", NULL);
    return 0;
}

command echo_cmd = {
    "echo", 
    echo_func, 
    "Echoes the input arguments",
    "ECHO(1)                     User Commands                     ECHO(1)\n\n"
    "NAME\n"
    "       echo - display a line of text\n\n"
    "SYNOPSIS\n"
    "       echo [STRING]...\n\n"
    "DESCRIPTION\n"
    "       Echo the STRINGs to standard output.\n"
    "       Multiple arguments are separated by single spaces.\n\n"
    "EXAMPLES\n"
    "       echo hello world\n"
    "       echo \"This is a test\"\n\n"
}; 