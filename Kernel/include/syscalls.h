#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
// Números de Syscall
#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_PUT_TEXT 2
#define SYSCALL_SET_FONT_SIZE 3
#define SYSCALL_DRAW_SQUARE 4
#define SYSCALL_GET_SCREEN_WIDTH 5
#define SYSCALL_GET_SCREEN_HEIGHT 6
#define SYSCALL_CLEAR_TEXT_BUFFER 7
#define SYSCALL_CLEAR_SCREEN 8
#define SYSCALL_BEEP 9
#define SYSCALL_PRINT_REGISTERS 10
#define SYSCALL_DATE 11
#define SYSCALL_RERENDER_CONSOLE 12
#define SYSCALL_DRAW_RECTANGLE 13
#define SYSCALL_GET_TICKS 14
#define SYSCALL_BIND 15
#define SYSCALL_UNBIND 16
#define SYSCALL_EXEC 17
#define SYSCALL_DRAW_CIRCLE 18
#define SYSCALL_IS_KEY_PRESSED 19
#define SYSCALL_GET_KEY_STATES 20
#define SYSCALL_ARE_ANY_KEYS_PRESSED 21
#define SYSCALL_ARE_ALL_KEYS_PRESSED 22
#define SYSCALL_RENDER_SCREEN 23

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);

// Nuevas syscalls para videoDriver
uint64_t sys_put_text_wrapper(uint64_t str_ptr, uint64_t len, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize);
uint64_t sys_set_font_size_wrapper(uint64_t fontSize);
uint64_t sys_draw_square_wrapper(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t size);
uint64_t sys_get_screen_width_wrapper(void);  // No necesita argumentos de registros directamente
uint64_t sys_get_screen_height_wrapper(void); // No necesita argumentos de registros directamente
uint64_t sys_clear_text_buffer_wrapper(void); // No necesita argumentos de registros directamente
uint64_t sys_clear_screen_wrapper(uint64_t clearColor);
uint64_t sys_draw_rectangle_wrapper(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t width, uint64_t height);
uint64_t sys_draw_circle_wrapper(uint64_t hexColor, uint64_t centerX, uint64_t centerY, uint64_t radius);
uint64_t sys_rerender_console_wrapper(void); // No necesita argumentos de registros directamente

// Syscall para imprimir registros
uint64_t sys_print_registers_wrapper(void);

uint64_t sys_beep_wrapper(uint64_t freq, uint64_t milis);

// Syscall para obtener la fecha y hora
uint64_t sys_read_rtc(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);

// Syscall para obtener los ticks elapsed
uint64_t sys_get_ticks_wrapper(void);

// Syscall para bind de funciones a scancodes
uint64_t sys_bind_wrapper(uint64_t scancode, uint64_t function_ptr);

// Syscall para unbind de funciones a scancodes
uint64_t sys_unbind_wrapper(uint64_t scancode);

// Syscall para ejecutar una dirección de memoria
uint64_t sys_exec_wrapper(uint64_t address);

// Syscalls para soporte multi-key
uint64_t sys_is_key_pressed_wrapper(uint64_t scancode);
uint64_t sys_get_key_states_wrapper(uint64_t scancodes_ptr, uint64_t states_ptr, uint64_t count);
uint64_t sys_are_any_keys_pressed_wrapper(uint64_t scancodes_ptr, uint64_t count);
uint64_t sys_are_all_keys_pressed_wrapper(uint64_t scancodes_ptr, uint64_t count);

// Syscall para renderizar pantalla (swap buffers)
uint64_t sys_render_screen_wrapper(void);

#endif