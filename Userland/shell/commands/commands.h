#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_ARGS 10

 
typedef struct
{
    char *name;
    int (*func)(int argc, char **argv);   
    char *description;
} command;

 
extern char *current_args[MAX_ARGS];
extern int arg_count;

 
extern command clear_cmd;
extern command help_cmd;
extern command test_synchro_cmd;
extern command test_no_synchro_cmd;
extern command test_processes_cmd;
extern command ps_cmd;
extern command loop_cmd;
extern command kill_cmd;
extern command nice_cmd;
extern command block_cmd;
extern command wc_cmd;
extern command cat_cmd;
extern command mem_cmd;
extern command filter_cmd;
extern command mvar_cmd;

 
extern command *all_commands[];

#endif  
