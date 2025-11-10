// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <videoDriver.h>
#include <font.h>
#include <lib.h>

// CONSTANTS

#define DEFAULT_FONT_SIZE 1
#define MAX_FONT_SIZE 5
#define MIN_FONT_SIZE 1

// VBE MODE INFORMATION STRUCTURE

struct vbe_mode_info_structure
{
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed));

typedef struct vbe_mode_info_structure *VBEInfoPtr;
// 0x5C00 is the address where the bootloader stores VBE information
VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;

static uint64_t font_size = DEFAULT_FONT_SIZE;

// Double buffering - always enabled
#define MAX_WIDTH 1024
#define MAX_HEIGHT 768
// 3 bytes per pixel (RGB) for maximum supported resolution
static uint8_t static_back_buffer[MAX_WIDTH * MAX_HEIGHT * 3];
static uint8_t *back_buffer = static_back_buffer;

void swap_buffers(void)
{
    uint8_t *framebuffer = (uint8_t *)(uint64_t)VBE_mode_info->framebuffer;
    uint64_t buffer_size = VBE_mode_info->width * VBE_mode_info->height * 3;

    // Fast memory copy of entire buffer using memcpy
    memcpy(framebuffer, back_buffer, buffer_size);
}

void clear_back_buffer(uint64_t color)
{
    uint8_t blue = color & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t red = (color >> 16) & 0xFF;

    for (uint64_t y = 0; y < VBE_mode_info->height; y++)
    {
        for (uint64_t x = 0; x < VBE_mode_info->width; x++)
        {
            uint64_t offset = (x * 3) + (y * VBE_mode_info->pitch);
            back_buffer[offset] = blue;
            back_buffer[offset + 1] = green;
            back_buffer[offset + 2] = red;
        }
    }
}

uint8_t *get_back_buffer(void)
{
    return back_buffer;
}

void put_pixel(uint64_t color, uint64_t x, uint64_t y)
{
    if (x >= VBE_mode_info->width || y >= VBE_mode_info->height)
    {
        return;
    }

    uint64_t offset = (x * 3) + (y * VBE_mode_info->pitch);
    back_buffer[offset] = color & 0xFF;             // Blue
    back_buffer[offset + 1] = (color >> 8) & 0xFF;  // Green
    back_buffer[offset + 2] = (color >> 16) & 0xFF; // Red
}

void draw_rect(uint64_t color, uint64_t x, uint64_t y, uint64_t width, uint64_t height)
{
    for (uint64_t row = 0; row < height; row++)
    {
        for (uint64_t col = 0; col < width; col++)
        {
            put_pixel(color, x + col, y + row);
        }
    }
}

void clear_screen(uint64_t color)
{
    clear_back_buffer(color);
}

void draw_char_with_size(char c, uint64_t color, uint64_t x, uint64_t y, uint64_t size)
{
    const uint8_t *char_data = FONT[(unsigned char)c];

    for (uint64_t row = 0; row < FONT_HEIGHT; row++)
    {
        uint8_t font_row = char_data[row];
        for (uint64_t col = 0; col < FONT_WIDTH; col++)
        {
            if (font_row & (1 << col))
            {
                // Draw scaled pixel
                for (uint64_t sy = 0; sy < size; sy++)
                {
                    for (uint64_t sx = 0; sx < size; sx++)
                    {
                        put_pixel(color, x + col * size + sx, y + row * size + sy);
                    }
                }
            }
        }
    }
}

void draw_char(char c, uint64_t color, uint64_t x, uint64_t y)
{
    draw_char_with_size(c, color, x, y, font_size);
}

uint64_t get_screen_width_pixels(void)
{
    return VBE_mode_info->width;
}

uint64_t get_screen_height_pixels(void)
{
    return VBE_mode_info->height;
}

uint64_t get_font_width(void)
{
    return font_size * FONT_WIDTH;
}

uint64_t get_font_height(void)
{
    return font_size * FONT_HEIGHT;
}

uint64_t get_chars_per_line(void)
{
    return VBE_mode_info->width / (font_size * FONT_WIDTH);
}