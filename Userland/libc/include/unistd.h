#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);

void sys_set_font_size(uint64_t size);

void sys_put_text(const char *text, uint64_t len, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize);

void sys_draw_square(uint64_t hexColor, uint64_t x, uint64_t y, uint64_t size);

void sys_draw_rectangle(uint64_t hexColor, uint64_t x, uint64_t y, uint64_t width, uint64_t height);

void sys_draw_circle(uint64_t hexColor, uint64_t centerX, uint64_t centerY, uint64_t radius);

uint64_t sys_get_screen_width(void);

uint64_t sys_get_screen_height(void);

void sys_clear_text_buffer(void);

void sys_beep(uint64_t freq, uint64_t milis);

void sys_print_registers(void);

void sys_clear_screen(uint64_t color);

void sys_rerender_console(void);

uint64_t sys_read_rtc(uint64_t rtc_register);

uint64_t sys_get_ticks(void);

int sys_bind(uint64_t scancode, void (*function_ptr)(void));
int sys_unbind(uint64_t scancode);

void sys_exec(uint64_t address);

// Multi-key support syscalls
uint8_t sys_is_key_pressed(uint8_t scancode);
uint64_t sys_get_key_states(uint8_t* scancodes, uint8_t* states, uint64_t count);
uint8_t sys_are_any_keys_pressed(uint8_t* scancodes, uint64_t count);
uint8_t sys_are_all_keys_pressed(uint8_t* scancodes, uint64_t count);

// Render screen (swap buffers)
void sys_render_screen(void);

#endif