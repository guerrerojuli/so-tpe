#ifndef CONSOLE_DRIVER_H
#define CONSOLE_DRIVER_H

#include <stdint.h>

// Console dimensions - should match video buffer
#define CONSOLE_WIDTH 200
#define CONSOLE_HEIGHT 150

// Console character structure - stores char and its color
typedef struct
{
    char c;
    uint32_t color;
} console_char_t;

// Writes text to console. Handles newlines, tabs, backspace automatically.
// Color format: 0xRRGGBB (RGB hex)
void console_write(const char *data, uint64_t data_len, uint64_t hexColor);

// Wipes console clean and resets cursor to top-left
void console_clear(void);

#endif