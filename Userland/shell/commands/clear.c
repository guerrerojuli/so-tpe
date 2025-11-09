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