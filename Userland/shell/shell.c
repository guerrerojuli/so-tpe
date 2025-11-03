#include "unistd.h"
#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "commands/commands.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_PIPE_COMMANDS 2

// Command parsing structure
typedef struct {
    char *name;
    char *args[MAX_ARGS];
    int arg_count;
} Command;

typedef struct {
    Command commands[MAX_PIPE_COMMANDS];
    int command_count;
    int is_background;
} ParsedInput;

// Global variables for backward compatibility with existing commands
char input_buffer[MAX_COMMAND_LENGTH];
char *current_args[MAX_ARGS];
int arg_count = 0;

// Import existing commands
extern command clear_cmd;
extern command echo_cmd;
extern command help_cmd;
extern command man_cmd;
extern command divzero_cmd;
extern command invop_cmd;
extern command test_sync_cmd;
extern command ps_cmd;
extern command test_pipes_cmd;

// Import new commands
extern command loop_cmd;
extern command cat_cmd;
extern command wc_cmd;
extern command filter_cmd;

// Array of all commands
command *all_commands[] = {
    &clear_cmd,
    &echo_cmd,
    &help_cmd,
    &man_cmd,
    &divzero_cmd,
    &invop_cmd,
    &test_sync_cmd,
    &ps_cmd,
    &test_pipes_cmd,
    &loop_cmd,
    &cat_cmd,
    &wc_cmd,
    &filter_cmd,
    NULL // Terminator
};

// Find command by name
command* find_command(char *name) {
    for (int i = 0; all_commands[i] != NULL; i++) {
        if (strcmp(name, all_commands[i]->name) == 0) {
            return all_commands[i];
        }
    }
    return NULL;
}

// Parse a single command (tokenize by spaces)
void parse_single_command(char *input, Command *cmd) {
    cmd->arg_count = 0;
    cmd->name = NULL;

    // Skip leading spaces
    while (*input == ' ' || *input == '\t') input++;

    if (*input == '\0') return;

    // Use strtok to parse
    char *token = strtok(input, " \t");
    if (token == NULL) return;

    cmd->name = token;
    cmd->args[cmd->arg_count++] = token;

    // Get remaining arguments
    while ((token = strtok(NULL, " \t")) != NULL && cmd->arg_count < MAX_ARGS - 1) {
        cmd->args[cmd->arg_count++] = token;
    }

    cmd->args[cmd->arg_count] = NULL;
}

// Parse input with pipe and background support
void parse_input(char *input, ParsedInput *parsed) {
    parsed->command_count = 0;
    parsed->is_background = 0;

    // Check for background operator at the end
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '&') {
        parsed->is_background = 1;
        input[len - 1] = '\0';  // Remove '&'
        len--;

        // Remove trailing spaces
        while (len > 0 && (input[len - 1] == ' ' || input[len - 1] == '\t')) {
            input[--len] = '\0';
        }
    }

    // Check for pipe operator
    char *pipe_pos = strchr(input, '|');
    if (pipe_pos != NULL) {
        *pipe_pos = '\0';  // Split at pipe

        // Parse first command
        parse_single_command(input, &parsed->commands[0]);
        if (parsed->commands[0].name != NULL) {
            parsed->command_count = 1;
        }

        // Parse second command (skip spaces after pipe)
        char *cmd2 = pipe_pos + 1;
        while (*cmd2 == ' ' || *cmd2 == '\t') cmd2++;

        if (*cmd2 != '\0') {
            parse_single_command(cmd2, &parsed->commands[1]);
            if (parsed->commands[1].name != NULL) {
                parsed->command_count = 2;
            }
        }
    } else {
        // Single command
        parse_single_command(input, &parsed->commands[0]);
        if (parsed->commands[0].name != NULL) {
            parsed->command_count = 1;
        }
    }
}

// Execute a single command (built-in)
void execute_builtin_command(Command *cmd, int is_background) {
    command *cmd_ptr = find_command(cmd->name);
    if (cmd_ptr == NULL) {
        printf("Command '%s' not found. Type 'help' for available commands.\n",
               (void **)&cmd->name);
        return;
    }

    // Set global args for backward compatibility
    arg_count = cmd->arg_count;
    for (int i = 0; i < arg_count; i++) {
        current_args[i] = cmd->args[i];
    }
    current_args[arg_count] = NULL;

    // For now, execute directly (no process creation yet for built-in commands)
    // TODO: When we have proper process creation, use create_process_with_fds
    if (is_background) {
        printf("Background execution not yet supported for built-in commands\n", NULL);
        cmd_ptr->func(arg_count, current_args);
    } else {
        cmd_ptr->func(arg_count, current_args);
    }
}

// Execute a single command with process creation
void execute_single_command(Command *cmd, int is_background) {
    command *cmd_ptr = find_command(cmd->name);
    if (cmd_ptr == NULL) {
        printf("Command '%s' not found. Type 'help' for available commands.\n",
               (void **)&cmd->name);
        return;
    }

    // Set global args for backward compatibility
    arg_count = cmd->arg_count;
    for (int i = 0; i < arg_count; i++) {
        current_args[i] = cmd->args[i];
    }
    current_args[arg_count] = NULL;

    if (is_background) {
        // Background process: redirect stdin to DEV_NULL
        int16_t fds[3] = {DEV_NULL, STDOUT, STDERR};
        int64_t pid = create_process_with_fds((void *)cmd_ptr->func,
                                              cmd->args, cmd->name,
                                              1, fds);  // Priority 1
        if (pid < 0) {
            // Process creation not supported, run directly
            printf("Note: Running directly (process creation not available)\n", NULL);
            cmd_ptr->func(arg_count, current_args);
        } else {
            int pid_int = (int)pid;
            void *args[] = {&pid_int};
            printf("[%d] Started in background\n", args);
        }
    } else {
        // Foreground process: normal stdin, wait for completion
        int16_t fds[3] = {STDIN, STDOUT, STDERR};
        int64_t pid = create_process_with_fds((void *)cmd_ptr->func,
                                              cmd->args, cmd->name,
                                              1, fds);  // Priority 1
        if (pid < 0) {
            // Process creation not supported, run directly
            cmd_ptr->func(arg_count, current_args);
        } else {
            waitpid((uint16_t)pid);  // Wait for completion
        }
    }
}

// Execute piped commands
void execute_piped_commands(ParsedInput *parsed) {
    command *cmd1_ptr = find_command(parsed->commands[0].name);
    command *cmd2_ptr = find_command(parsed->commands[1].name);

    if (cmd1_ptr == NULL) {
        printf("Command '%s' not found.\n", (void **)&parsed->commands[0].name);
        return;
    }
    if (cmd2_ptr == NULL) {
        printf("Command '%s' not found.\n", (void **)&parsed->commands[1].name);
        return;
    }

    // Create pipe
    int16_t pipe_id = pipe_get();
    if (pipe_id < 0) {
        printf("Failed to create pipe (may not be implemented yet)\n", NULL);
        // Fallback: execute sequentially
        execute_single_command(&parsed->commands[0], 0);
        execute_single_command(&parsed->commands[1], 0);
        return;
    }

    // First process: stdout -> pipe
    int16_t fds1[3] = {
        parsed->is_background ? DEV_NULL : STDIN,
        pipe_id,  // stdout to pipe
        STDERR
    };

    // Set global args for first command
    arg_count = parsed->commands[0].arg_count;
    for (int i = 0; i < arg_count; i++) {
        current_args[i] = parsed->commands[0].args[i];
    }

    int64_t pid1 = create_process_with_fds((void *)cmd1_ptr->func,
                                           parsed->commands[0].args,
                                           parsed->commands[0].name,
                                           1, fds1);

    if (pid1 < 0) {
        printf("Failed to create first process in pipe\n", NULL);
        return;
    }

    // Second process: stdin <- pipe
    int16_t fds2[3] = {
        pipe_id,  // stdin from pipe
        STDOUT,
        STDERR
    };

    // Set global args for second command
    arg_count = parsed->commands[1].arg_count;
    for (int i = 0; i < arg_count; i++) {
        current_args[i] = parsed->commands[1].args[i];
    }

    int64_t pid2 = create_process_with_fds((void *)cmd2_ptr->func,
                                           parsed->commands[1].args,
                                           parsed->commands[1].name,
                                           1, fds2);

    if (pid2 < 0) {
        printf("Failed to create second process in pipe\n", NULL);
        // TODO: Should kill first process?
        return;
    }

    if (parsed->is_background) {
        int p1 = (int)pid1;
        int p2 = (int)pid2;
        void *pargs[2] = {&p1, &p2};
        printf("[%d,%d] Pipeline started in background\n", pargs);
    } else {
        // Wait for first process (it will block until pipe is consumed)
        waitpid((uint16_t)pid1);
    }
}

// Main shell loop
void shell_loop(void) {
    ParsedInput parsed;

    while (1) {
        // Show prompt
        printf("$ ", NULL);

        // Read input
        if (fgets(input_buffer, MAX_COMMAND_LENGTH, STDIN) != NULL) {
            // Remove newline
            size_t len = strlen(input_buffer);
            if (len > 0 && input_buffer[len - 1] == '\n') {
                input_buffer[len - 1] = '\0';
                len--;
            }

            // Skip empty lines
            if (len == 0) continue;

            // Parse input
            parse_input(input_buffer, &parsed);

            // Execute based on command structure
            if (parsed.command_count == 2) {
                // Piped commands
                execute_piped_commands(&parsed);
            } else if (parsed.command_count == 1) {
                // Single command
                execute_single_command(&parsed.commands[0],
                                      parsed.is_background);
            }
        } else {
            // EOF or error
            printf("\nExiting shell...\n", NULL);
            break;
        }
    }
}

int main() {
    printf("Simple Shell v2.0\n", NULL);
    printf("Features: Background (&), Pipes (|), Ctrl+C, Ctrl+D\n", NULL);
    printf("Type 'help' for available commands\n\n", NULL);
    shell_loop();
    return 0;
}