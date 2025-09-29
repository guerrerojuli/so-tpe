#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <videoDriver.h>
#include <font.h>
#include <lib.h>

//=============================================================================
// CONSTANTS
//=============================================================================

#define DEFAULT_FONT_SIZE 1
#define MAX_FONT_SIZE 5
#define MIN_FONT_SIZE 1

//=============================================================================
// VBE MODE INFORMATION STRUCTURE
//=============================================================================

struct vbe_mode_info_structure {
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
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;
// 0x5C00 is the address where the bootloader stores VBE information
VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

//=============================================================================
// GLOBAL STATE
//=============================================================================

static uint64_t font_size = DEFAULT_FONT_SIZE;

// Double buffering - always enabled
#define MAX_WIDTH 1024
#define MAX_HEIGHT 768
// 3 bytes per pixel (RGB) for maximum supported resolution
static uint8_t static_back_buffer[MAX_WIDTH * MAX_HEIGHT * 3];
static uint8_t* back_buffer = static_back_buffer;

//=============================================================================
// DOUBLE BUFFERING FUNCTIONS
//=============================================================================

/**
 * Swap buffers - copy back buffer to front buffer
 */
void swap_buffers(void) {
    uint8_t* framebuffer = (uint8_t*)VBE_mode_info->framebuffer;
    uint64_t buffer_size = VBE_mode_info->width * VBE_mode_info->height * 3;
    
    // Fast memory copy of entire buffer using memcpy
    memcpy(framebuffer, back_buffer, buffer_size);
}

/**
 * Clear the back buffer
 */
void clear_back_buffer(uint64_t color) {
    uint8_t blue = color & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t red = (color >> 16) & 0xFF;
    
    for (uint64_t y = 0; y < VBE_mode_info->height; y++) {
        for (uint64_t x = 0; x < VBE_mode_info->width; x++) {
            uint64_t offset = (x * 3) + (y * VBE_mode_info->pitch);
            back_buffer[offset] = blue;
            back_buffer[offset + 1] = green;
            back_buffer[offset + 2] = red;
        }
    }
}

/**
 * Get direct access to back buffer for advanced operations
 */
uint8_t* get_back_buffer(void) {
    return back_buffer;
}

//=============================================================================
// BASIC DRAWING FUNCTIONS
//=============================================================================

/**
 * Draw a single pixel to back buffer
 */
void put_pixel(uint64_t color, uint64_t x, uint64_t y) {
    if (x >= VBE_mode_info->width || y >= VBE_mode_info->height) {
        return;
    }
    
    uint64_t offset = (x * 3) + (y * VBE_mode_info->pitch);
    back_buffer[offset] = color & 0xFF;           // Blue
    back_buffer[offset + 1] = (color >> 8) & 0xFF;   // Green
    back_buffer[offset + 2] = (color >> 16) & 0xFF;  // Red
}

/**
 * Draw a rectangle
 */
void draw_rect(uint64_t color, uint64_t x, uint64_t y, uint64_t width, uint64_t height) {
    for (uint64_t row = 0; row < height; row++) {
        for (uint64_t col = 0; col < width; col++) {
            put_pixel(color, x + col, y + row);
        }
    }
}

/**
 * Draw a square
 */
void draw_square(uint64_t color, uint64_t x, uint64_t y, uint64_t size) {
    draw_rect(color, x, y, size, size);
}

// Draw a filled circle using midpoint circle algorithm
void draw_circle(uint64_t color, uint64_t centerX, uint64_t centerY, uint64_t radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw horizontal lines to fill the circle
        // Top and bottom halves
        draw_hline(color, centerX - x, centerY + y, 2 * x + 1);
        draw_hline(color, centerX - x, centerY - y, 2 * x + 1);
        
        // Don't duplicate the center line when y = 0
        if (y != 0) {
            draw_hline(color, centerX - y, centerY + x, 2 * y + 1);
            draw_hline(color, centerX - y, centerY - x, 2 * y + 1);
        }

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/**
 * Clear the screen (back buffer)
 */
void clear_screen(uint64_t color) {
    clear_back_buffer(color);
}

//=============================================================================
// TEXT RENDERING FUNCTIONS
//=============================================================================

/**
 * Draw a character with specified size
 */
void draw_char_with_size(char c, uint64_t color, uint64_t x, uint64_t y, uint64_t size) {
    const uint8_t* char_data = FONT[(unsigned char)c];
    
    for (uint64_t row = 0; row < FONT_HEIGHT; row++) {
        uint8_t font_row = char_data[row];
        for (uint64_t col = 0; col < FONT_WIDTH; col++) {
            if (font_row & (1 << col)) {
                // Draw scaled pixel
                for (uint64_t sy = 0; sy < size; sy++) {
                    for (uint64_t sx = 0; sx < size; sx++) {
                        put_pixel(color, x + col * size + sx, y + row * size + sy);
                    }
                }
            }
        }
    }
}

/**
 * Draw a character with current font size
 */
void draw_char(char c, uint64_t color, uint64_t x, uint64_t y) {
    draw_char_with_size(c, color, x, y, font_size);
}

/**
 * Draw a string with specified size
 */
void draw_string_with_size(const char* str, uint64_t len, uint64_t color, uint64_t x, uint64_t y, uint64_t size) {
    if (str == NULL) {
        return;
    }
    
    uint64_t char_width = size * FONT_WIDTH;
    uint64_t current_x = x;
    
    for (uint64_t i = 0; i < len; i++) {
        draw_char_with_size(str[i], color, current_x, y, size);
        current_x += char_width;
    }
}

/**
 * Draw a string with current font size
 */
void draw_string(const char* str, uint64_t len, uint64_t color, uint64_t x, uint64_t y) {
    draw_string_with_size(str, len, color, x, y, font_size);
}

//=============================================================================
// DIMENSION FUNCTIONS
//=============================================================================

uint64_t get_screen_width_pixels(void) {
    return VBE_mode_info->width;
}

uint64_t get_screen_height_pixels(void) {
    return VBE_mode_info->height;
}

uint64_t get_font_width(void) {
    return font_size * FONT_WIDTH;
}

uint64_t get_font_height(void) {
    return font_size * FONT_HEIGHT;
}

uint64_t get_chars_per_line(void) {
    return VBE_mode_info->width / (font_size * FONT_WIDTH);
}

//=============================================================================
// FONT MANAGEMENT
//=============================================================================

void set_font_size(uint64_t new_font_size) {
    if (new_font_size >= MIN_FONT_SIZE && new_font_size <= MAX_FONT_SIZE) {
        font_size = new_font_size;
    }
}

uint64_t get_font_size(void) {
    return font_size;
}

//=============================================================================
// SIMPLE LINE FUNCTIONS
//=============================================================================

void draw_hline(uint64_t color, uint64_t x, uint64_t y, uint64_t width) {
    draw_rect(color, x, y, width, 1);
}

void draw_vline(uint64_t color, uint64_t x, uint64_t y, uint64_t height) {
    draw_rect(color, x, y, 1, height);
}