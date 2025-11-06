#include <registers.h>
#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <stddef.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {
    [SYSCALL_READ] = sys_read,
    [SYSCALL_WRITE] = sys_write,
    [SYSCALL_CREATE_PROCESS] = sys_create_process,
    [SYSCALL_KILL_PROCESS] = sys_kill_process,
    [SYSCALL_GET_PID] = sys_get_pid,
    [SYSCALL_YIELD] = sys_yield,
    [SYSCALL_SET_PRIORITY] = sys_set_priority,
    [SYSCALL_CLEAR_TEXT_BUFFER] = sys_clear_text_buffer_wrapper,
    [SYSCALL_BLOCK] = sys_block,
    [SYSCALL_MALLOC] = sys_malloc,
    [SYSCALL_FREE] = sys_free,
    [SYSCALL_MEM_STATE] = sys_mem_state,
    [SYSCALL_SEM_INIT] = sys_sem_init,
    [SYSCALL_SEM_OPEN] = sys_sem_open,
    [SYSCALL_SEM_CLOSE] = sys_sem_close,
    [SYSCALL_SEM_DESTROY] = sys_sem_destroy,
    [SYSCALL_SEM_WAIT] = sys_sem_wait,
    [SYSCALL_SEM_POST] = sys_sem_post,
    [SYSCALL_WAITPID] = sys_waitpid,
    [SYSCALL_PIPE_OPEN] = sys_pipe_open,
    [SYSCALL_PIPE_CLOSE] = sys_pipe_close,
    [SYSCALL_PIPE_GET] = sys_pipe_get,
    [SYSCALL_GET_PROCESS_INFO] = sys_get_process_info,
    [SYSCALL_SLEEP] = sys_sleep,
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