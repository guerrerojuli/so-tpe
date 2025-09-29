#include "stdio.h"
#include "stddef.h"
#include "unistd.h"
#include "commands.h"

static int clear_func(void) {
    sys_clear_text_buffer();
    return 0;
}

command clear_cmd = {
    "clear", 
    clear_func, 
    "Clears the screen", 
    "CLEAR(1)                    User Commands                    CLEAR(1)\n\n"
    "NAME\n"
    "       clear - clear the terminal screen\n\n"
    "SYNOPSIS\n"
    "       clear\n\n"
    "DESCRIPTION\n"
    "       Clear the terminal screen using ANSI escape sequences.\n"
    "       This command does not take any arguments.\n\n"
}; 