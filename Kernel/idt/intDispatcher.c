#include <registers.h>
#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <scheduler.h>
#include <stddef.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {
    [SYSCALL_READ] = (uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))sys_read,
    [SYSCALL_WRITE] = (uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))sys_write,
    [SYSCALL_CLEAR_TEXT_BUFFER] = (uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))sys_clear_text_buffer_wrapper,
    [SYSCALL_YIELD] = (uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))sys_yield,
    [SYSCALL_EXIT] = (uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))sys_exit,
};

uint64_t intDispatcher(const registers_t *registers)
{
    uint64_t syscall_num = registers->rax;

    if (syscall_num < sizeof(intHandlers) / sizeof(intHandlers[0]) && intHandlers[syscall_num] != NULL)
    {
        uint64_t ret = intHandlers[syscall_num](
            registers->rdi,
            registers->rsi,
            registers->rdx,
            registers->rcx,
            registers->r8,
            registers->r9);

        // Handle cooperative scheduling for yield/exit syscalls
        if (syscall_num == SYSCALL_YIELD || syscall_num == SYSCALL_EXIT)
        {
            uint64_t current_rsp = (uint64_t)registers;
            uint64_t next_rsp = scheduler_switch(current_rsp);
            return next_rsp; // non-zero triggers ASM to switch stacks
        }

        return ret;
    }

    return 0;
}