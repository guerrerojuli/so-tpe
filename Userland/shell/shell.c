#include "unistd.h"
#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "commands/commands.h"
#define MAX_COMMAND_LENGTH 256

// Variables globales para parsear comandos
char input_buffer[MAX_COMMAND_LENGTH];
char *current_args[MAX_ARGS];
int arg_count = 0;

// Importamos los comandos de los módulos separados
extern command clear_cmd;
extern command echo_cmd;
extern command help_cmd;
extern command man_cmd;
extern command divzero_cmd;
extern command invop_cmd;
extern command test_sync_cmd;
extern command ps_cmd;

// Array de todos los comandos
command *all_commands[] = {
    &clear_cmd,
    &echo_cmd,
    &help_cmd,
    &man_cmd,
    &divzero_cmd,
    &invop_cmd,
    &test_sync_cmd,
    &ps_cmd,
    NULL // Terminator
};

// Función para parsear la línea de comando
void parse_command(char *input)
{
    arg_count = 0;

    // First call to strtok with the input string
    char *token = strtok(input, " \t");

    // Continue tokenizing until no more tokens or max args reached
    while (token != NULL && arg_count < MAX_ARGS - 1)
    {
        current_args[arg_count++] = token;
        token = strtok(NULL, " \t"); // Subsequent calls with NULL
    }

    current_args[arg_count] = NULL;
}

// Función para ejecutar un comando
int execute_command(char *cmd_name)
{
    for (int i = 0; all_commands[i] != NULL; i++)
    {
        if (strcmp(cmd_name, all_commands[i]->name) == 0)
        {
            return all_commands[i]->func();
        }
    }

    // Comando no encontrado
    printf("Command '%s' not found. Type 'help' for available commands.\n", (void **)&cmd_name);
    return -1;
}

// Función principal de la shell
void shell_loop(void)
{
    while (1)
    {
        // Mostrar prompt
        printf("$ ", NULL);

        // Leer input del usuario
        if (fgets(input_buffer, MAX_COMMAND_LENGTH, STDIN) != NULL)
        {
            // Remove the newline character if present
            size_t len = strlen(input_buffer);
            if (len > 0 && input_buffer[len - 1] == '\n')
            {
                input_buffer[len - 1] = '\0';
            }
        }
        else
        {
            // If fgets fails, set empty string
            input_buffer[0] = '\0';
        }

        // Si la línea está vacía, continuar
        if (strlen(input_buffer) == 0)
        {
            continue;
        }

        // Parsear el comando
        parse_command(input_buffer);

        if (arg_count > 0)
        {
            // Ejecutar el comando
            execute_command(current_args[0]);
        }
    }
}

int main()
{
    printf("Simple Shell v1.0\n", NULL);
    printf("Type 'help' for available commands\n", NULL);
    printf("To quit, close QEMU (Ctrl+A X in terminal)\n\n", NULL);
    shell_loop();

    return 0;
}
