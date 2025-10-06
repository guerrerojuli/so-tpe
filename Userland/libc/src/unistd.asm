GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_clear_text_buffer
GLOBAL sys_create_process
GLOBAL sys_kill_process
GLOBAL sys_get_pid
GLOBAL sys_yield
GLOBAL sys_set_priority
GLOBAL sys_block

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

; Syscall numbers must match Kernel/include/syscalls.h
; SYSCALL_READ = 0
; SYSCALL_WRITE = 1
; SYSCALL_CREATE_PROCESS = 2
; SYSCALL_KILL_PROCESS = 3
; SYSCALL_GET_PID = 4
; SYSCALL_YIELD = 5
; SYSCALL_SET_PRIORITY = 6
; SYSCALL_CLEAR_TEXT_BUFFER = 7
; SYSCALL_BLOCK = 8

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

