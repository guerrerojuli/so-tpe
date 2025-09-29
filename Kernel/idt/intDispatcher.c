#include <registers.h>
#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <stddef.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {
    [SYSCALL_READ] = sys_read,
    [SYSCALL_WRITE] = sys_write,
    [SYSCALL_CLEAR_TEXT_BUFFER] = sys_clear_text_buffer_wrapper,
};

uint64_t intDispatcher(const registers_t *registers)
{
    uint64_t syscall_num = registers->rax;

    if (syscall_num < sizeof(intHandlers) / sizeof(intHandlers[0]) && intHandlers[syscall_num] != NULL)
    {
        return intHandlers[syscall_num](
            registers->rdi,
            registers->rsi,
            registers->rdx,
            registers->rcx,
            registers->r8,
            registers->r9);
    }

    return 0;
}