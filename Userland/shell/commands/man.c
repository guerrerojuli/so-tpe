#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "commands.h"

static int man_func(void) {
    if (arg_count < 2) {
        printf("Usage: man <command>\n", NULL);
        printf("Available manual pages:\n", NULL);
        for (int i = 0; all_commands[i] != NULL; i++) {
            void *args[1] = {all_commands[i]->name};
            printf("  %s\n", args);
        }
        return 0;
    }
    
    char *cmd_name = current_args[1];
    
    // Buscar el comando en el array
    for (int i = 0; all_commands[i] != NULL; i++) {
        if (strcmp(cmd_name, all_commands[i]->name) == 0) {
            if (all_commands[i]->manual != NULL) {
                printf("%s", (void**)&all_commands[i]->manual);
                return 0;
            } else {
                void *args[1] = {cmd_name};
                printf("No manual entry available for %s\n", args);
                return -1;
            }
        }
    }
    
    // Comando no encontrado
    void *args[1] = {cmd_name};
    printf("No manual entry for %s\n", args);
    printf("Use 'man' without arguments to see available manual pages.\n", NULL);
    return -1;
}

command man_cmd = {
    "man", 
    man_func, 
    "Shows manual pages for commands",
    "MAN(1)                      User Commands                      MAN(1)\n\n"
    "NAME\n"
    "       man - display manual pages\n\n"
    "SYNOPSIS\n"
    "       man <command>\n\n"
    "DESCRIPTION\n"
    "       Display the manual page for the specified command.\n"
    "       If no command is specified, show available manual pages.\n\n"
    "EXAMPLES\n"
    "       man echo\n"
    "       man help\n\n"
}; 