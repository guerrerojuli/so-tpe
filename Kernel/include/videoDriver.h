#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

void swap_buffers(void);
void clear_back_buffer(uint64_t color);
uint8_t *get_back_buffer(void);

void put_pixel(uint64_t hexColor, uint64_t x, uint64_t y);
void draw_rect(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t width, uint64_t height);
void clear_screen(uint64_t clearColor);

void draw_char(char c, uint64_t hexColor, uint64_t posX, uint64_t posY);
void draw_char_with_size(char c, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize);

uint64_t get_screen_width_pixels(void);
uint64_t get_screen_height_pixels(void);

uint64_t get_font_width(void);
uint64_t get_font_height(void);
uint64_t get_chars_per_line(void);

#endif
