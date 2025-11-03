#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

// File descriptors
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define DEV_NULL -1
#define BUILT_IN_DESCRIPTORS 3

// Pipe modes
#define READ 0
#define WRITE 1

// Process states
typedef enum {
    READY = 0,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessStatus;

// Process entry point signature
typedef int (*MainFunction)(int argc, char **args);

#endif
