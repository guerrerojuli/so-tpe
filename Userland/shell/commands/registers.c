#include "commands.h"
#include "unistd.h"

static int registers_func(void) {
    sys_print_registers();
    return 0;
}

command registers_cmd = {
    "registers", 
    registers_func, 
    "Shows saved registers",
    "REGISTERS(1)                     User Commands                     REGISTERS(1)\n\n"
    "NAME\n"
    "       registers - display saved registers\n\n"
    "       save registers pressing ESC\n\n"
    "SYNOPSIS\n"
    "       registers\n\n"
    "DESCRIPTION\n"
    "       Display the saved registers.\n"
}; 