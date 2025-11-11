

#include <stdint.h>
#include "../include/unistd.h"
#include "../include/stddef.h"
#include "unistd.h"

uintptr_t __stack_chk_guard = 0xDEADBEEFDEADBEEFUL;

void __attribute__((noreturn)) __stack_chk_fail(void)
{

    char error_msg[] = "*** USERLAND: Stack corruption detected! Terminating program ***\n";
    sys_write(2, error_msg, sizeof(error_msg) - 1);

    while (1)
        ;
}