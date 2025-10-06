#include <syscalls.h>
#include <videoDriver.h>
#include <consoleDriver.h>
#include <keyboardDriver.h>
#include <lib.h>
#include <scheduler.h>

enum
{
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2
};

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count)
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

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count)
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

// Syscall 2: sys_clear_text_buffer
// Clears the console buffer
uint64_t sys_clear_text_buffer_wrapper(void)
{
    console_clear();
    return 0;
}

uint64_t sys_yield(void)
{
    // Return special non-zero to signal ASM to switch to returned RSP
    // We pass current stack pointer context via intDispatcher; here just return 0 and let dispatcher set RAX
    // The actual switch will be decided in intDispatcher where we have access to registers
    return 0;
}

uint64_t sys_exit(void)
{
    scheduler_task_exit();
    return 0;
}