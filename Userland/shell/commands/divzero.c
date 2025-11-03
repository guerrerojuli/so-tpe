#include "stdio.h"
#include "stddef.h"
#include "unistd.h"
#include "commands.h"

static int divzero_func(int argc, char **argv) {
    printf("Triggering divide by zero exception...\n", NULL);
    
    // Provocar una división por cero
    volatile int x = 1;
    volatile int y = 0;
    volatile int result = x / y;  // Esto causará la excepción
    
    // Esta línea nunca se ejecutará
    printf("Result: %d\n", (void**)&result);
    return 0;
}

command divzero_cmd = {
    "divzero", 
    divzero_func, 
    "Triggers a divide by zero exception for testing", 
    "DIVZERO(1)                  User Commands                  DIVZERO(1)\n\n"
    "NAME\n"
    "       divzero - trigger a divide by zero exception\n\n"
    "SYNOPSIS\n"
    "       divzero\n\n"
    "DESCRIPTION\n"
    "       This command intentionally triggers a divide by zero exception\n"
    "       to test the exception handling system. The exception handler\n"
    "       will display the registers and exception information.\n\n"
    "WARNING\n"
    "       This command will cause a system exception and may halt\n"
    "       the system depending on the exception handler implementation.\n\n"
}; 