#include "stdio.h"
#include "stddef.h"
#include "unistd.h"
#include "commands.h"

static int invop_func(int argc, char **argv) {
    printf("Triggering invalid opcode exception...\n", NULL);
    
    // Array con el opcode inválido UD2 (0x0F 0x0B)
    static unsigned char invalid_code[] = {0x0F, 0x0B, 0xC3}; // UD2 + RET
    
    // Convertir el array a puntero de función y ejecutar
    void (*invalid_func)(void) = (void(*)(void))invalid_code;
    invalid_func();
    
    // Esta línea nunca se ejecutará
    printf("This line should never be reached\n", NULL);
    return 0;
}

command invop_cmd = {
    "invop", 
    invop_func, 
    "Triggers an invalid opcode exception for testing", 
    "INVOP(1)                    User Commands                    INVOP(1)\n\n"
    "NAME\n"
    "       invop - trigger an invalid opcode exception\n\n"
    "SYNOPSIS\n"
    "       invop\n\n"
    "DESCRIPTION\n"
    "       This command intentionally triggers an invalid opcode exception\n"
    "       using the UD2 instruction (undefined instruction) to test the\n"
    "       exception handling system. The exception handler will display\n"
    "       the registers and exception information.\n\n"
    "WARNING\n"
    "       This command will cause a system exception and may halt\n"
    "       the system depending on the exception handler implementation.\n\n"
}; 