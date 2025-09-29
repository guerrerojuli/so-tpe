GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_clear_text_buffer

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

sys_clear_text_buffer:
    syscall 2

