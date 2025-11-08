#include <syscalls.h>
#include <videoDriver.h>
#include <consoleDriver.h>
#include <keyboardDriver.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <semaphoreManager.h>
#include <lib.h>
#include <pipe.h>
#include <time.h>

// I/O syscalls
uint64_t sys_read(uint64_t fd, char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3)
{
    // Get actual file descriptor from process's FD table
    int16_t actual_fd = get_process_fd((uint8_t)fd);

    // If the process doesn't have this FD (shouldn't happen), fail
    if (actual_fd == -1 && fd < 3) {
        return 0;
    }

    // For FDs >= 3, they're direct pipe descriptors, not indirected
    if (fd >= 3) {
        actual_fd = (int16_t)fd;
    }

    // Check if it's a pipe
    if (actual_fd >= BUILT_IN_DESCRIPTORS) {
        return pipe_read((uint16_t)actual_fd, buf, count);
    }

    // Handle DEV_NULL
    if (actual_fd == DEV_NULL) {
        return 0;  // Reading from DEV_NULL returns EOF immediately
    }

    switch (actual_fd)
    {
    case STDIN:
        int i = 0;
        for (i = 0; i < count; i++)
        {
            // Use blocking read - will wait until a character is available
            buf[i] = getCharBlocking();

            // Check if we got EOF (Ctrl+D returns -1)
            if ((int8_t)buf[i] == -1) {
                // Return immediately with bytes read including EOF
                return i + 1;
            }
        }
        return i;
    default:
        return 0;
    }
}

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3)
{
    // Get actual file descriptor from process's FD table
    int16_t actual_fd = get_process_fd((uint8_t)fd);

    // If the process doesn't have this FD (shouldn't happen), fail
    if (actual_fd == -1 && fd < 3) {
        return 0;
    }

    // For FDs >= 3, they're direct pipe descriptors, not indirected
    if (fd >= 3) {
        actual_fd = (int16_t)fd;
    }

    // Check if it's a pipe
    if (actual_fd >= BUILT_IN_DESCRIPTORS) {
        return pipe_write(get_pid(), (uint16_t)actual_fd, buf, count);
    }

    // Handle DEV_NULL
    if (actual_fd == DEV_NULL) {
        return count;  // Discard all output, pretend we wrote it
    }

    switch (actual_fd)
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

uint64_t sys_unblock(uint64_t pid, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    int8_t result = set_status((uint16_t)pid, READY);
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

uint64_t sys_mem_state(uint64_t total_ptr, uint64_t free_ptr, uint64_t used_ptr, uint64_t name_ptr, uint64_t _unused1, uint64_t _unused2)
{
    uint64_t total, free, used;
    mm_get_stats(&total, &free);

    // For buddy system, values are in pages, convert to bytes
    #ifdef BUDDY
    total *= 4096;  // PAGE_SIZE
    free *= 4096;
    #endif

    used = total - free;

    if (total_ptr)
        *((uint64_t*)total_ptr) = total;
    if (free_ptr)
        *((uint64_t*)free_ptr) = free;
    if (used_ptr)
        *((uint64_t*)used_ptr) = used;
    if (name_ptr) {
        const char* name = mm_get_name();
        // Copy the name to userland memory
        char* dest = (char*)name_ptr;
        int i = 0;
        while (name[i] != '\0' && i < 31) {  // Max 31 chars + null
            dest[i] = name[i];
            i++;
        }
        dest[i] = '\0';
    }

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

// Pipe syscalls
uint64_t sys_pipe_open(uint64_t id, uint64_t mode, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    int8_t result = pipe_open_for_pid(get_pid(), (uint16_t)id, (uint8_t)mode);
    return (uint64_t)result;
}

uint64_t sys_pipe_close(uint64_t id, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    int8_t result = pipe_close_for_pid(get_pid(), (uint16_t)id);
    return (uint64_t)result;
}

uint64_t sys_pipe_get(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6)
{
    int16_t result = pipe_get();
    return (uint64_t)result;
}

uint64_t sys_get_process_info(uint64_t info_array_ptr, uint64_t max_count, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4)
{
    ProcessInfo *info_array = (ProcessInfo *)info_array_ptr;
    int32_t result = get_process_info(info_array, (uint32_t)max_count);
    return (uint64_t)result;
}

uint64_t sys_sleep(uint64_t seconds, uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5)
{
    // Calculate target ticks (18 ticks per second)
    uint64_t ticks_to_wait = seconds * 18;
    uint64_t target_ticks = ticks_elapsed() + ticks_to_wait;

    // Yield CPU until target time is reached
    while (ticks_elapsed() < target_ticks) {
        yield();  // Give up CPU to other processes
    }

    return 0;
}

uint64_t sys_get_ticks(uint64_t _unused1, uint64_t _unused2, uint64_t _unused3, uint64_t _unused4, uint64_t _unused5, uint64_t _unused6)
{
    return (uint64_t)ticks_elapsed();
}