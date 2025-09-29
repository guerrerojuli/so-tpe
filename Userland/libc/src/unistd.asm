GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_set_font_size
GLOBAL sys_put_text
GLOBAL sys_draw_square
GLOBAL sys_draw_rectangle
GLOBAL sys_draw_circle
GLOBAL sys_get_screen_width
GLOBAL sys_get_screen_height
GLOBAL sys_clear_text_buffer
GLOBAL sys_beep
GLOBAL sys_print_registers
GLOBAL sys_clear_screen
GLOBAL sys_read_rtc
GLOBAL sys_rerender_console
GLOBAL sys_get_ticks
GLOBAL sys_bind
GLOBAL sys_unbind
GLOBAL sys_exec
GLOBAL sys_is_key_pressed
GLOBAL sys_get_key_states
GLOBAL sys_are_any_keys_pressed
GLOBAL sys_are_all_keys_pressed
GLOBAL sys_render_screen

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

sys_put_text:
    syscall 2

sys_set_font_size:
    syscall 3

sys_draw_square:
    syscall 4

sys_draw_rectangle:
    syscall 13

sys_draw_circle:
    syscall 18

sys_get_screen_width:
    syscall 5

sys_get_screen_height:
    syscall 6

sys_clear_text_buffer:
    syscall 7

sys_clear_screen:
    syscall 8

sys_beep:
    syscall 9

sys_print_registers:
    syscall 10

sys_read_rtc:
    syscall 11

sys_rerender_console:
    syscall 12

sys_get_ticks:
    syscall 14

sys_bind:
    syscall 15

sys_unbind:
    syscall 16

sys_exec:
    syscall 17

sys_is_key_pressed:
    syscall 19

sys_get_key_states:
    syscall 20

sys_are_any_keys_pressed:
    syscall 21

sys_are_all_keys_pressed:
    syscall 22

sys_render_screen:
    syscall 23
