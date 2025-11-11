// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "include/pipe.h"
#include "include/scheduler.h"
#include "include/memoryManager.h"
#include "include/lib.h"
#include "include/globals.h"
#include <stddef.h>

static PipeManager *pipeManager;

#define bufferPosition(pipe) (((pipe)->startPosition + (pipe)->currentSize) % PIPE_SIZE)

static int16_t get_pipe_index_by_id(uint16_t id);
static Pipe *get_pipe_by_id(uint16_t id);
static Pipe *create_pipe(void);
static void free_pipe(Pipe *pipe);

static int16_t get_pipe_index_by_id(uint16_t id)
{
    int16_t index = (int16_t)id - BUILT_IN_DESCRIPTORS;
    if (index < 0 || index >= MAX_PIPES)
    {
        return -1;
    }
    return index;
}

static Pipe *get_pipe_by_id(uint16_t id)
{
    int16_t index = get_pipe_index_by_id(id);
    if (index == -1)
    {
        return NULL;
    }
    return pipeManager->pipes[index];
}

static Pipe *create_pipe(void)
{
    Pipe *pipe = (Pipe *)mm_alloc(sizeof(Pipe));
    if (pipe == NULL)
    {
        return NULL;
    }

    pipe->startPosition = 0;
    pipe->currentSize = 0;
    pipe->inputPid = -1;
    pipe->outputPid = -1;
    pipe->isBlocking = 0;
    memset(pipe->buffer, 0, PIPE_SIZE);

    return pipe;
}

static void free_pipe(Pipe *pipe)
{
    mm_free(pipe);
}

void pipe_manager_init()
{
    pipeManager = (PipeManager *)PIPE_MANAGER_ADDRESS;
    pipeManager->lastFreePipe = 0;
    pipeManager->qtyPipes = 0;

    for (int i = 0; i < MAX_PIPES; i++)
    {
        pipeManager->pipes[i] = NULL;
    }
}

int16_t pipe_get()
{
    if (pipeManager->qtyPipes >= MAX_PIPES)
    {
        return -1;
    }

    while (pipeManager->pipes[pipeManager->lastFreePipe] != NULL)
    {
        pipeManager->lastFreePipe = (pipeManager->lastFreePipe + 1) % MAX_PIPES;
    }

    Pipe *newPipe = create_pipe();
    if (newPipe == NULL)
    {
        return -1;
    }

    pipeManager->pipes[pipeManager->lastFreePipe] = newPipe;
    pipeManager->qtyPipes++;

    int16_t pipeId = pipeManager->lastFreePipe + BUILT_IN_DESCRIPTORS;
    pipeManager->lastFreePipe = (pipeManager->lastFreePipe + 1) % MAX_PIPES;

    return pipeId;
}

int8_t pipe_open_for_pid(uint16_t pid, uint16_t id, uint8_t mode)
{
    int16_t index = get_pipe_index_by_id(id);
    if (index == -1)
    {
        return -1;
    }

    Pipe *pipe = pipeManager->pipes[index];

    if (pipe == NULL)
    {
        if (pipeManager->qtyPipes >= MAX_PIPES)
        {
            return -1;
        }

        pipe = create_pipe();
        if (pipe == NULL)
        {
            return -1;
        }

        pipeManager->pipes[index] = pipe;
        pipeManager->qtyPipes++;
    }

    if (mode == WRITE)
    {
        if (pipe->inputPid != -1 && pipe->inputPid != (int16_t)pid)
        {
            return -1;
        }
        pipe->inputPid = (int16_t)pid;

        if (pipe->outputPid != -1 && pipe->isBlocking)
        {
            set_status(pipe->outputPid, READY);
            pipe->isBlocking = 0;
        }
    }
    else if (mode == READ)
    {
        if (pipe->outputPid != -1 && pipe->outputPid != (int16_t)pid)
        {
            return -1;
        }
        pipe->outputPid = (int16_t)pid;

        if (pipe->inputPid != -1 && pipe->isBlocking)
        {
            set_status(pipe->inputPid, READY);
            pipe->isBlocking = 0;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int8_t pipe_close_for_pid(uint16_t pid, uint16_t id)
{
    int16_t index = get_pipe_index_by_id(id);
    if (index == -1)
    {
        return -1;
    }

    Pipe *pipe = pipeManager->pipes[index];
    if (pipe == NULL)
    {
        return -1;
    }

    if (pid == pipe->inputPid)
    {
        char eofString[1] = {EOF_MARKER};
        pipe_write(pid, id, eofString, 1);
        pipe->inputPid = -1;

        if (pipe->isBlocking && pipe->outputPid != -1)
        {
            set_status(pipe->outputPid, READY);
            pipe->isBlocking = 0;
        }
    }

    else if (pid == pipe->outputPid)
    {
        pipe->outputPid = -1;

        if (pipe->isBlocking && pipe->inputPid != -1)
        {
            set_status(pipe->inputPid, READY);
            pipe->isBlocking = 0;
        }

        free_pipe(pipe);
        pipeManager->pipes[index] = NULL;
        pipeManager->qtyPipes--;
    }
    else
    {
        return -1;
    }

    return 0;
}

int64_t pipe_read(uint16_t id, char *buffer, uint64_t len)
{
    Pipe *pipe = get_pipe_by_id(id);
    if (pipe == NULL || pipe->outputPid != (int16_t)get_pid() || len == 0)
    {
        return -1;
    }

    uint64_t readBytes = 0;
    uint8_t eofRead = 0;

    while (pipe->inputPid == -1 &&
           pipe->currentSize == 0 &&
           (int)pipe->buffer[pipe->startPosition] != EOF_MARKER)
    {
        pipe->isBlocking = 1;
        set_status(get_pid(), BLOCKED);
        yield();

        if (pipe != get_pipe_by_id(id))
        {
            return -1;
        }
    }

    while (readBytes < len && !eofRead)
    {

        if (pipe->currentSize == 0 && (int)pipe->buffer[pipe->startPosition] != EOF_MARKER)
        {
            pipe->isBlocking = 1;
            set_status(get_pid(), BLOCKED);
            yield();
        }

        while ((pipe->currentSize > 0 || (int)pipe->buffer[pipe->startPosition] == EOF_MARKER) && readBytes < len)
        {
            buffer[readBytes] = pipe->buffer[pipe->startPosition];

            if ((int)buffer[readBytes++] == EOF_MARKER)
            {
                eofRead = 1;
                break;
            }

            pipe->currentSize--;
            pipe->startPosition = (pipe->startPosition + 1) % PIPE_SIZE;
        }

        if (pipe->isBlocking)
        {
            set_status(pipe->inputPid, READY);
            pipe->isBlocking = 0;
        }
    }

    return readBytes;
}

int64_t pipe_write(uint16_t pid, uint16_t id, const char *buffer, uint64_t len)
{
    Pipe *pipe = get_pipe_by_id(id);
    if (pipe == NULL || pipe->inputPid != (int16_t)pid || len == 0)
    {
        return -1;
    }

    uint64_t writtenBytes = 0;

    while (writtenBytes < len && (int)pipe->buffer[bufferPosition(pipe)] != EOF_MARKER)
    {

        if (pipe->currentSize >= PIPE_SIZE)
        {
            pipe->isBlocking = 1;
            set_status(pid, BLOCKED);
            yield();
        }

        if (pipe != get_pipe_by_id(id))
        {
            return -1;
        }

        while (pipe->currentSize < PIPE_SIZE && writtenBytes < len)
        {
            pipe->buffer[bufferPosition(pipe)] = buffer[writtenBytes];

            if ((int)buffer[writtenBytes++] == EOF_MARKER)
            {
                break;
            }

            pipe->currentSize++;
        }

        if (pipe->isBlocking)
        {
            set_status(pipe->outputPid, READY);
            pipe->isBlocking = 0;
        }
    }

    return writtenBytes;
}
