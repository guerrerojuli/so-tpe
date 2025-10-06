#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

// File descriptors
#define STDIN 0
#define STDOUT 1
#define STDERR 2

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
