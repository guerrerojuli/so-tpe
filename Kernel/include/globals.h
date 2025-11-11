#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define DEV_NULL -1
#define BUILT_IN_DESCRIPTORS 3

#define READ 0
#define WRITE 1

typedef enum
{
    READY = 0,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessStatus;

typedef int (*MainFunction)(int argc, char **args);

#endif
