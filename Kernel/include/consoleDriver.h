#ifndef CONSOLE_DRIVER_H
#define CONSOLE_DRIVER_H

#include <stdint.h>

// Console dimensions - should match video buffer
#define CONSOLE_WIDTH 200   
#define CONSOLE_HEIGHT 150  

// Console character structure - stores char and its color
typedef struct {
    char c;
    uint32_t color;
} console_char_t;

// Main console functions

// Writes text to console. Handles newlines, tabs, backspace automatically.
// Color format: 0xRRGGBB (RGB hex)
void console_write(const char* data, uint64_t data_len, uint64_t hexColor);

// Wipes console clean and resets cursor to top-left
void console_clear(void);

// Changes font size (1-5) and redraws everything
void console_set_font_size(uint64_t fontSize);

// Get current console dimensions in characters (not pixels)
uint64_t console_get_width(void);
uint64_t console_get_height(void);

// Cursor management - tracks where next character will appear
uint64_t console_get_cursor_x(void);  // Column position
uint64_t console_get_cursor_y(void);  // Row position  
void console_set_cursor(uint64_t x, uint64_t y);  // Move cursor (simplified for now)

// Rendering control - usually called automatically
void console_rerender(void);  // Force complete redraw
void console_flush(void);     // No-op, kept for compatibility

#endif 