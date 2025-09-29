#include <registers.h>
#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <soundDriver.h>
#include <stddef.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {
    [SYSCALL_READ] = sys_read,
    [SYSCALL_WRITE] = sys_write,
    [SYSCALL_PUT_TEXT] = sys_put_text_wrapper,
    [SYSCALL_SET_FONT_SIZE] = sys_set_font_size_wrapper,
    [SYSCALL_DRAW_SQUARE] = sys_draw_square_wrapper,
    [SYSCALL_GET_SCREEN_WIDTH] = sys_get_screen_width_wrapper,
    [SYSCALL_GET_SCREEN_HEIGHT] = sys_get_screen_height_wrapper,
    [SYSCALL_CLEAR_TEXT_BUFFER] = sys_clear_text_buffer_wrapper,
    [SYSCALL_CLEAR_SCREEN] = sys_clear_screen_wrapper,
    [SYSCALL_BEEP] = sys_beep_wrapper,
    [SYSCALL_PRINT_REGISTERS] = sys_print_registers_wrapper,
    [SYSCALL_DATE] = sys_read_rtc,
    [SYSCALL_RERENDER_CONSOLE] = sys_rerender_console_wrapper,
    [SYSCALL_DRAW_RECTANGLE] = sys_draw_rectangle_wrapper,
    [SYSCALL_GET_TICKS] = sys_get_ticks_wrapper,
    [SYSCALL_BIND] = sys_bind_wrapper,
    [SYSCALL_UNBIND] = sys_unbind_wrapper,
    [SYSCALL_EXEC] = sys_exec_wrapper,
    [SYSCALL_DRAW_CIRCLE] = sys_draw_circle_wrapper,
    [SYSCALL_IS_KEY_PRESSED] = sys_is_key_pressed_wrapper,
    [SYSCALL_GET_KEY_STATES] = sys_get_key_states_wrapper,
    [SYSCALL_ARE_ANY_KEYS_PRESSED] = sys_are_any_keys_pressed_wrapper,
    [SYSCALL_ARE_ALL_KEYS_PRESSED] = sys_are_all_keys_pressed_wrapper,
    [SYSCALL_RENDER_SCREEN] = sys_render_screen_wrapper};

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