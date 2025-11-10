// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include "../include/unistd.h"
#include "../include/stddef.h"
#include "unistd.h"

// Stack canary global variable for stack protection
uintptr_t __stack_chk_guard = 0xDEADBEEFDEADBEEFUL;

// Stack check failure function for userland - terminates the program
void __attribute__((noreturn)) __stack_chk_fail(void)
{
    // Write error message to stderr
    char error_msg[] = "*** USERLAND: Stack corruption detected! Terminating program ***\n";
    sys_write(2, error_msg, sizeof(error_msg) - 1);
    
    // Exit the program immediately
    //sys_exit(1);
    while (1);
} 