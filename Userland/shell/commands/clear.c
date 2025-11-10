// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdio.h"
#include "stddef.h"
#include "unistd.h"
#include "commands.h"

static int clear_func(int argc, char **argv) {
    sys_clear_text_buffer();
    return 0;
}

command clear_cmd = {
    "clear",
    clear_func,
    "Clears the screen"
}; 