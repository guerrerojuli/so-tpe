#ifndef PIPE_H
#define PIPE_H

#include <stdint.h>

#define PIPE_SIZE 4096
#define MAX_PIPES 4096
#define PIPE_MANAGER_ADDRESS 0x80000
#define EOF_MARKER -1

typedef struct Pipe {
    char buffer[PIPE_SIZE];
    uint16_t startPosition;
    uint16_t currentSize;
    int16_t inputPid;
    int16_t outputPid;
    uint8_t isBlocking;
} Pipe;

typedef struct PipeManager {
    Pipe *pipes[MAX_PIPES];
    uint16_t lastFreePipe;
    uint16_t qtyPipes;
} PipeManager;

// Pipe manager functions
void pipe_manager_init();
int16_t pipe_get();
int8_t pipe_open_for_pid(uint16_t pid, uint16_t id, uint8_t mode);
int8_t pipe_close_for_pid(uint16_t pid, uint16_t id);
int64_t pipe_read(uint16_t id, char *buffer, uint64_t len);
int64_t pipe_write(uint16_t pid, uint16_t id, const char *buffer, uint64_t len);

#endif
