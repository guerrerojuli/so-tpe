#include <syscalls.h>
#include <videoDriver.h>
#include <consoleDriver.h>
#include <keyboardDriver.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <semaphoreManager.h>
#include <lib.h>

// I/O syscalls
uint64_t sys_read(uint64_t fd, char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3)
{
    switch (fd)
    {
    case STDIN:
        int i = 0;
        while (i < count)
        {
            char c = getChar();
            if (c == 0)
                continue;
            buf[i++] = c;
        }
        return i;
    default:
        return 0;
    }
}

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3)
{
    switch (fd)
    {
    case STDOUT:
        console_write(buf, count, 0xFFFFFF);
        return count;
    case STDERR:
        console_write(buf, count, 0xFF0000);
        return count;
    default:
        return 0;
    }
}

uint64_t sys_clear_text_buffer_wrapper(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6)
{
    console_clear();
    return 0;
}

// Process management syscalls
uint64_t sys_create_process(uint64_t code_ptr, uint64_t args_ptr, uint64_t name_ptr, uint64_t priority, uint64_t fds_ptr, uint64_t _unused)
{
    MainFunction code = (MainFunction)code_ptr;
    char **args = (char **)args_ptr;
    char *name = (char *)name_ptr;
    int16_t *fds = (int16_t *)fds_ptr;

    // Validate priority
    if (priority >= NUM_PRIORITIES)
        return -1;

    int16_t pid = create_process(code, args, name, (uint8_t)priority, fds, 0);
    return (uint64_t)pid;
}

uint64_t sys_kill_process(uint64_t pid, uint64_t retval, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    int32_t result = kill_process((uint16_t)pid, (int32_t)retval);
    return (uint64_t)result;
}

uint64_t sys_get_pid(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6)
{
    return (uint64_t)get_pid();
}

uint64_t sys_yield(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6)
{
    yield();
    return 0;
}

uint64_t sys_set_priority(uint64_t pid, uint64_t new_priority, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    // Validate priority
    if (new_priority >= NUM_PRIORITIES)
        return -1;

    int8_t result = set_priority((uint16_t)pid, (uint8_t)new_priority);
    return (uint64_t)result;
}

uint64_t sys_block(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    int8_t result = set_status((uint16_t)pid, BLOCKED);
    return (uint64_t)result;
}

// Memory management syscalls
uint64_t sys_malloc(uint64_t size, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    return (uint64_t)mm_alloc((uint32_t)size);
}

uint64_t sys_free(uint64_t ptr, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    mm_free((void*)ptr);
    return 0;
}

uint64_t sys_mem_state(uint64_t total_ptr, uint64_t free_ptr, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    uint64_t total, free;
    mm_get_stats(&total, &free);

    if (total_ptr)
        *((uint64_t*)total_ptr) = total;
    if (free_ptr)
        *((uint64_t*)free_ptr) = free;

    return 0;
}

// Semaphore syscalls
uint64_t sys_sem_init(uint64_t sem_id, uint64_t initial_value, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_init(&id, (uint32_t)initial_value);
    return (uint64_t)result;
}

uint64_t sys_sem_open(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_open(&id);
    return (uint64_t)result;
}

uint64_t sys_sem_close(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_close(&id);
    return (uint64_t)result;
}

uint64_t sys_sem_destroy(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_destroy(&id);
    return (uint64_t)result;
}

uint64_t sys_sem_wait(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_wait(&id);
    return (uint64_t)result;
}

uint64_t sys_sem_post(uint64_t sem_id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    sem_t id = (sem_t)sem_id;
    int8_t result = sem_post(&id);
    return (uint64_t)result;
}

uint64_t sys_waitpid(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    int32_t result = waitpid((uint16_t)pid);
    return (uint64_t)result;
}