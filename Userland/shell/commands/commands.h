#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_ARGS 10

// Estructura para definir comandos
typedef struct
{
    char *name;
    int (*func)(void);
    char *description;
    char *manual;
} command;

// Variables globales para parsear comandos (definidas en shell.c)
extern char *current_args[MAX_ARGS];
extern int arg_count;

// Declaraciones de funciones de comandos
extern command clear_cmd;
extern command echo_cmd;
extern command help_cmd;
extern command man_cmd;
extern command divzero_cmd;
extern command invop_cmd;
extern command test_sync_cmd;
extern command ps_cmd;
extern command test_pipes_cmd;

// Array de todos los comandos (definido en shell.c)
extern command *all_commands[];

#endif /* COMMANDS_H */
