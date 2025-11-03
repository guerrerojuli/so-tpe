#include "include/pipe.h"
#include "include/scheduler.h"
#include "include/memoryManager.h"
#include "include/lib.h"
#include "include/globals.h"
#include <stddef.h>

static PipeManager *pipeManager;

#define bufferPosition(pipe) (((pipe)->startPosition + (pipe)->currentSize) % PIPE_SIZE)

void pipe_manager_init() {
    pipeManager = (PipeManager *)PIPE_MANAGER_ADDRESS;
    pipeManager->lastFreePipe = 0;
    pipeManager->qtyPipes = 0;

    for (int i = 0; i < MAX_PIPES; i++) {
        pipeManager->pipes[i] = NULL;
    }
}

int16_t pipe_get() {
    if (pipeManager->qtyPipes >= MAX_PIPES) {
        return -1;
    }

    // Find next free slot
    while (pipeManager->pipes[pipeManager->lastFreePipe] != NULL) {
        pipeManager->lastFreePipe = (pipeManager->lastFreePipe + 1) % MAX_PIPES;
    }

    // Allocate new pipe
    Pipe *newPipe = (Pipe *)mm_alloc(sizeof(Pipe));
    if (newPipe == NULL) {
        return -1;
    }

    // Initialize pipe
    newPipe->startPosition = 0;
    newPipe->currentSize = 0;
    newPipe->inputPid = -1;
    newPipe->outputPid = -1;
    newPipe->isBlocking = 0;
    memset(newPipe->buffer, 0, PIPE_SIZE);

    // Store in manager
    pipeManager->pipes[pipeManager->lastFreePipe] = newPipe;
    pipeManager->qtyPipes++;

    // Return pipe ID
    int16_t pipeId = pipeManager->lastFreePipe + BUILT_IN_DESCRIPTORS;
    pipeManager->lastFreePipe = (pipeManager->lastFreePipe + 1) % MAX_PIPES;

    return pipeId;
}

int8_t pipe_open_for_pid(uint16_t pid, uint16_t id, uint8_t mode) {
    if (id < BUILT_IN_DESCRIPTORS || id >= BUILT_IN_DESCRIPTORS + MAX_PIPES) {
        return -1;
    }

    uint16_t index = id - BUILT_IN_DESCRIPTORS;
    Pipe *pipe = pipeManager->pipes[index];

    // Lazy pipe creation: create pipe if it doesn't exist
    if (pipe == NULL) {
        // Check if we can create more pipes
        if (pipeManager->qtyPipes >= MAX_PIPES) {
            return -1;
        }

        // Allocate new pipe
        pipe = (Pipe *)mm_alloc(sizeof(Pipe));
        if (pipe == NULL) {
            return -1;
        }

        // Initialize pipe
        pipe->startPosition = 0;
        pipe->currentSize = 0;
        pipe->inputPid = -1;
        pipe->outputPid = -1;
        pipe->isBlocking = 0;
        memset(pipe->buffer, 0, PIPE_SIZE);

        // Store in manager
        pipeManager->pipes[index] = pipe;
        pipeManager->qtyPipes++;
    }

    if (mode == WRITE) {
        if (pipe->inputPid != -1 && pipe->inputPid != pid) {
            return -1;
        }
        pipe->inputPid = pid;

        // Wake up reader if it's waiting for writer
        if (pipe->outputPid != -1 && pipe->isBlocking) {
            set_status(pipe->outputPid, READY);
            pipe->isBlocking = 0;
        }
    } else if (mode == READ) {
        if (pipe->outputPid != -1 && pipe->outputPid != pid) {
            return -1;
        }
        pipe->outputPid = pid;

        // Wake up writer if it's waiting for reader
        if (pipe->inputPid != -1 && pipe->isBlocking) {
            set_status(pipe->inputPid, READY);
            pipe->isBlocking = 0;
        }
    } else {
        return -1;
    }

    return 0;
}

int8_t pipe_close_for_pid(uint16_t pid, uint16_t id) {
    if (id < BUILT_IN_DESCRIPTORS || id >= BUILT_IN_DESCRIPTORS + MAX_PIPES) {
        return -1;
    }

    uint16_t index = id - BUILT_IN_DESCRIPTORS;
    Pipe *pipe = pipeManager->pipes[index];

    if (pipe == NULL) {
        return -1;
    }

    // If closing as writer, send EOF
    if (pipe->inputPid == pid) {
        pipe->buffer[bufferPosition(pipe)] = EOF_MARKER;
        pipe->inputPid = -1;

        // Wake reader if blocked
        if (pipe->isBlocking && pipe->outputPid != -1) {
            set_status(pipe->outputPid, READY);
            pipe->isBlocking = 0;
        }
    }

    // If closing as reader, free pipe
    if (pipe->outputPid == pid) {
        pipe->outputPid = -1;

        // Free pipe if no reader/writer
        if (pipe->inputPid == -1 && pipe->outputPid == -1) {
            mm_free(pipe);
            pipeManager->pipes[index] = NULL;
            pipeManager->qtyPipes--;
        }
    }

    return 0;
}

int64_t pipe_read(uint16_t id, char *buffer, uint64_t len) {
    if (id < BUILT_IN_DESCRIPTORS || id >= BUILT_IN_DESCRIPTORS + MAX_PIPES) {
        return -1;
    }

    uint16_t index = id - BUILT_IN_DESCRIPTORS;
    Pipe *pipe = pipeManager->pipes[index];

    if (pipe == NULL || pipe->outputPid != get_pid()) {
        return -1;
    }

    // Wait for a writer to connect or for data to be available
    while (pipe->inputPid == -1 &&
           pipe->currentSize == 0 &&
           pipe->buffer[pipe->startPosition] != EOF_MARKER) {
        pipe->isBlocking = 1;
        set_status(get_pid(), BLOCKED);
        yield();
    }
    pipe->isBlocking = 0;

    uint64_t readBytes = 0;
    uint8_t eofRead = 0;

    while (readBytes < len && !eofRead) {
        // Block if buffer empty and writer exists
        while (pipe->currentSize == 0 &&
               pipe->buffer[pipe->startPosition] != EOF_MARKER &&
               pipe->inputPid != -1) {
            pipe->isBlocking = 1;
            set_status(get_pid(), BLOCKED);
            yield();
        }

        // Check for EOF
        if (pipe->buffer[pipe->startPosition] == EOF_MARKER) {
            eofRead = 1;
            break;
        }

        // Read available data
        while (readBytes < len && pipe->currentSize > 0) {
            buffer[readBytes++] = pipe->buffer[pipe->startPosition];
            pipe->startPosition = (pipe->startPosition + 1) % PIPE_SIZE;
            pipe->currentSize--;

            // Unblock writer if it was blocked
            if (pipe->isBlocking && pipe->inputPid != -1) {
                set_status(pipe->inputPid, READY);
                pipe->isBlocking = 0;
            }
        }

        // If no writer, stop
        if (pipe->inputPid == -1) {
            break;
        }
    }

    return readBytes;
}

int64_t pipe_write(uint16_t pid, uint16_t id, const char *buffer, uint64_t len) {
    if (id < BUILT_IN_DESCRIPTORS || id >= BUILT_IN_DESCRIPTORS + MAX_PIPES) {
        return -1;
    }

    uint16_t index = id - BUILT_IN_DESCRIPTORS;
    Pipe *pipe = pipeManager->pipes[index];

    if (pipe == NULL || pipe->inputPid != pid) {
        return -1;
    }

    uint64_t writtenBytes = 0;
    uint16_t pos = bufferPosition(pipe);

    while (writtenBytes < len && pipe->buffer[pos] != EOF_MARKER) {
        // Block if buffer full and reader exists
        while (pipe->currentSize >= PIPE_SIZE && pipe->outputPid != -1) {
            pipe->isBlocking = 1;
            set_status(pid, BLOCKED);
            yield();
        }

        // Write available space
        while (writtenBytes < len && pipe->currentSize < PIPE_SIZE) {
            pos = bufferPosition(pipe);

            if (pipe->buffer[pos] == EOF_MARKER) {
                break;
            }

            pipe->buffer[pos] = buffer[writtenBytes++];
            pipe->currentSize++;

            // Unblock reader if it was blocked
            if (pipe->isBlocking && pipe->outputPid != -1) {
                set_status(pipe->outputPid, READY);
                pipe->isBlocking = 0;
            }
        }

        // If no reader, stop (broken pipe)
        if (pipe->outputPid == -1) {
            return -1;
        }
    }

    return writtenBytes;
}
