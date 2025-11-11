#ifndef CONSOLE_DRIVER_H
#define CONSOLE_DRIVER_H

#include <stdint.h>

#define CONSOLE_WIDTH 200
#define CONSOLE_HEIGHT 150

typedef struct
{
    char c;
    uint32_t color;
} console_char_t;

void console_write(const char *data, uint64_t data_len, uint64_t hexColor);

void console_clear(void);

#endif