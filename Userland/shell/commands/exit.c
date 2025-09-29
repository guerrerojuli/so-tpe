#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int exit_func(void) {
    printf("Goodbye!\n", NULL);
    return 1; // Retorna 1 para indicar que debe salir
}

command exit_cmd = {
    "exit", 
    exit_func, 
    "Exits the shell",
    "EXIT(1)                     User Commands                     EXIT(1)\n\n"
    "NAME\n"
    "       exit - exit the shell\n\n"
    "SYNOPSIS\n"
    "       exit\n\n"
    "DESCRIPTION\n"
    "       Terminate the shell session and return control to the system.\n"
    "       This command does not take any arguments.\n\n"
}; 