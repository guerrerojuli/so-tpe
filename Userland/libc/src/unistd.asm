GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_clear_text_buffer
GLOBAL sys_create_process
GLOBAL sys_kill_process
GLOBAL sys_get_pid
GLOBAL sys_yield
GLOBAL sys_set_priority
GLOBAL sys_block
GLOBAL sys_unblock
GLOBAL sys_malloc
GLOBAL sys_free
GLOBAL sys_sem_init
GLOBAL sys_sem_open
GLOBAL sys_sem_close
GLOBAL sys_sem_destroy
GLOBAL sys_sem_wait
GLOBAL sys_sem_post
GLOBAL sys_waitpid
GLOBAL sys_pipe_open
GLOBAL sys_pipe_close
GLOBAL sys_pipe_get
GLOBAL sys_get_process_info
GLOBAL sys_sleep
GLOBAL sys_mem_state
GLOBAL sys_get_ticks

section .text

%macro syscall 1
    push rbp
    mov rbp, rsp

    mov rax, %1
    mov r10, rcx
    int 0x80

    mov rsp, rbp
    pop rbp
    ret
%endmacro


























sys_read:
    syscall 0

sys_write:
    syscall 1

sys_create_process:
    syscall 2

sys_kill_process:
    syscall 3

sys_get_pid:
    syscall 4

sys_yield:
    syscall 5

sys_set_priority:
    syscall 6

sys_clear_text_buffer:
    syscall 7

sys_block:
    syscall 8

sys_malloc:
    syscall 9

sys_free:
    syscall 10

sys_mem_state:
    syscall 11

sys_unblock:
    syscall 24

sys_sem_init:
    syscall 12

sys_sem_open:
    syscall 13

sys_sem_close:
    syscall 14

sys_sem_destroy:
    syscall 15

sys_sem_wait:
    syscall 16

sys_sem_post:
    syscall 17

sys_waitpid:
    syscall 18

sys_pipe_open:
    syscall 19

sys_pipe_close:
    syscall 20

sys_pipe_get:
    syscall 21

sys_get_process_info:
    syscall 22

sys_sleep:
    syscall 23

sys_get_ticks:
    syscall 25


section .note.GNU-stack noalloc noexec nowrite progbits

