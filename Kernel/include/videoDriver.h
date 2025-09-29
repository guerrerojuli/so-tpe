#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>



// Double buffering - eliminates screen flicker
void swap_buffers(void);
void clear_back_buffer(uint64_t color);
uint8_t* get_back_buffer(void);  // Direct buffer access for optimizations

// Basic drawing primitives
void put_pixel(uint64_t hexColor, uint64_t x, uint64_t y);
void draw_rect(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t width, uint64_t height);
void draw_square(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t size);
void draw_circle(uint64_t hexColor, uint64_t centerX, uint64_t centerY, uint64_t radius);
void clear_screen(uint64_t clearColor);

// Text rendering - handles font scaling automatically
void draw_char(char c, uint64_t hexColor, uint64_t posX, uint64_t posY);
void draw_char_with_size(char c, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize);
void draw_string(const char* str, uint64_t len, uint64_t hexColor, uint64_t posX, uint64_t posY);
void draw_string_with_size(const char* str, uint64_t len, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize);

// Screen properties
uint64_t get_screen_width_pixels(void);
uint64_t get_screen_height_pixels(void);

// Font management
void set_font_size(uint64_t fontSize);
uint64_t get_font_size(void);
uint64_t get_font_width(void);    // Returns actual pixel width based on current size
uint64_t get_font_height(void);   // Returns actual pixel height based on current size
uint64_t get_chars_per_line(void); // How many characters fit horizontally

// Simple line drawing
void draw_hline(uint64_t hexColor, uint64_t x, uint64_t y, uint64_t width);
void draw_vline(uint64_t hexColor, uint64_t x, uint64_t y, uint64_t height);

#endif
