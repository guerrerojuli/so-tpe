// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "../include/unistd.h"
#include "../include/stddef.h"

int64_t create_process_with_fds(void *code, char **args, char *name,
                                uint8_t priority, int16_t fds[3])
{
    return sys_create_process((uint64_t)code, (uint64_t)args,
                              (uint64_t)name, (uint64_t)priority,
                              (uint64_t)fds);
}

int64_t waitpid(uint16_t pid)
{
    return sys_waitpid((uint64_t)pid);
}

int16_t pipe_get(void)
{
    return (int16_t)sys_pipe_get();
}