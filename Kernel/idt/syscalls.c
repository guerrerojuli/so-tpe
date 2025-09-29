#include <syscalls.h>
#include <videoDriver.h>
#include <consoleDriver.h>
#include <keyboardDriver.h>
#include <registers.h>
#include <soundDriver.h>
#include <lib.h>
#include <time.h>

enum
{
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2
};

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count)
{
    switch (fd)
    {
    case STDIN:
        int i = 0;
        while (i < count)
        {
            char c = getChar();
            if (c == 0)
                continue;
            buf[i++] = c;
        }
        return i;
    default:
        return 0;
    }
}

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count)
{
    switch (fd)
    {
    case STDOUT:
        console_write(buf, count, 0xFFFFFF);
        return count;
    case STDERR:
        console_write(buf, count, 0xFF0000);
        return count;
    default:
        return 0;
    }
}

// --- Syscall wrappers for videoDriver (direct graphics) ---

// Syscall 2: sys_put_text
// Arguments: rdi = const char* str, rsi = uint32_t len, rdx = uint32_t hexColor, rcx = uint32_t posX, r8 = uint32_t posY, r9 = uint32_t fontSize
// Direct graphics call - bypasses console for positioned text with custom font size
uint64_t sys_put_text_wrapper(uint64_t str_ptr, uint64_t len, uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t fontSize)
{
    draw_string_with_size((const char *)str_ptr, (uint32_t)len, (uint32_t)hexColor, (uint32_t)posX, (uint32_t)posY, (uint32_t)fontSize);
    return 0;
}

// Syscall 3: sys_set_font_size
// Arguments: rdi = uint32_t fontSize
// Uses console interface to set font size with proper re-rendering
uint64_t sys_set_font_size_wrapper(uint64_t fontSize)
{
    console_set_font_size((uint32_t)fontSize);
    return 0;
}

// Syscall 4: sys_draw_square
// Arguments: rdi = uint32_t hexColor, rsi = uint32_t posX, rdx = uint32_t posY, rcx = uint32_t size
// Direct graphics call - bypasses console for drawing shapes
uint64_t sys_draw_square_wrapper(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t size)
{
    draw_square((uint32_t)hexColor, (uint32_t)posX, (uint32_t)posY, (uint32_t)size);
    return 0;
}

// Syscall 13: sys_draw_rectangle
// Arguments: rdi = uint32_t hexColor, rsi = uint32_t posX, rdx = uint32_t posY, rcx = uint32_t width, r8 = uint32_t height
// Direct graphics call - bypasses console for drawing rectangles
uint64_t sys_draw_rectangle_wrapper(uint64_t hexColor, uint64_t posX, uint64_t posY, uint64_t width, uint64_t height)
{
    draw_rect((uint32_t)hexColor, (uint32_t)posX, (uint32_t)posY, (uint32_t)width, (uint32_t)height);
    return 0;
}

// Syscall 18: sys_draw_circle
// Arguments: rdi = uint32_t hexColor, rsi = uint32_t centerX, rdx = uint32_t centerY, rcx = uint32_t radius
// Direct graphics call - draws circle to back buffer
uint64_t sys_draw_circle_wrapper(uint64_t hexColor, uint64_t centerX, uint64_t centerY, uint64_t radius)
{
    draw_circle((uint32_t)hexColor, (uint32_t)centerX, (uint32_t)centerY, (uint32_t)radius);
    return 0;
}

// Syscall 5: sys_get_screen_width
// Returns console width in characters
uint64_t sys_get_screen_width_wrapper(void)
{
    return get_screen_width_pixels();
}

// Syscall 6: sys_get_screen_height
// Returns console height in characters
uint64_t sys_get_screen_height_wrapper(void)
{
    return get_screen_height_pixels();
}

// Syscall 7: sys_clear_text_buffer
// Clears the console buffer
uint64_t sys_clear_text_buffer_wrapper(void)
{
    console_clear();
    return 0;
}

// Syscall 8: sys_clear_screen
// Arguments: rdi = uint32_t clearColor
// Direct graphics call - clears entire screen with color
uint64_t sys_clear_screen_wrapper(uint64_t clearColor)
{
    clear_screen((uint32_t)clearColor);
    return 0;
}

// Syscall 11: sys_rerender_console
// No arguments
// Re-renders the entire console text buffer to the screen
uint64_t sys_rerender_console_wrapper(void)
{
    console_rerender();
    return 0;
}

// --- Print registers implementation ---

// Syscall 10: sys_print_registers_wrapper

uint64_t sys_print_registers_wrapper()
{
    print_stored_registers();
    return 0;
}

uint64_t sys_beep_wrapper(uint64_t freq, uint64_t milis)
{
    beep(freq, milis);
    return 0;
}

// Syscall 11: sys_read_rtc
uint64_t sys_read_rtc(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9)
{
    // Read RTC data directly like readRTC does
    // rdi contains the RTC register to read
    outb(0x70, (uint8_t)rdi);
    uint8_t bcd_value = inb(0x71);

    // Convert BCD to binary: (tens * 10) + ones
    uint8_t binary_value = ((bcd_value >> 4) * 10) + (bcd_value & 0x0F);

    return (uint64_t)binary_value;
}

// Syscall 14: sys_get_ticks
// Returns the number of timer ticks elapsed since boot
uint64_t sys_get_ticks_wrapper(void)
{
    return (uint64_t)ticks_elapsed();
}

// Syscall 15: sys_bind
// Arguments: rdi = uint8_t scancode, rsi = void (*function_ptr)(void)
// Binds a function to a scancode key
uint64_t sys_bind_wrapper(uint64_t scancode, uint64_t function_ptr)
{
    int result = bind_function((uint8_t)scancode, (void (*)(void))function_ptr);
    return (uint64_t)result;
}

// Syscall 16: sys_unbind
// Arguments: rdi = uint8_t scancode
// Unbinds a function from a scancode key, restoring normal functionality
uint64_t sys_unbind_wrapper(uint64_t scancode)
{
    int result = unbind_function((uint8_t)scancode);
    return (uint64_t)result;
}

// Syscall 17: sys_exec
// Arguments: rdi = uint64_t address
// Executes code at the specified memory address
uint64_t sys_exec_wrapper(uint64_t address)
{
    // Create a function pointer from the address and call it
    void (*func_ptr)(void) = (void (*)(void))address;
    func_ptr();
    return 0;
}

// Syscall 18: sys_is_key_pressed
// Arguments: rdi = uint8_t scancode
// Returns 1 if key is pressed, 0 if not pressed
uint64_t sys_is_key_pressed_wrapper(uint64_t scancode)
{
    return (uint64_t)is_key_pressed((uint8_t)scancode);
}

// Syscall 19: sys_get_key_states
// Arguments: rdi = uint8_t* scancodes, rsi = uint8_t* states, rdx = uint64_t count
// Returns number of keys successfully checked
uint64_t sys_get_key_states_wrapper(uint64_t scancodes_ptr, uint64_t states_ptr, uint64_t count)
{
    return get_key_states((uint8_t *)scancodes_ptr, (uint8_t *)states_ptr, count);
}

// Syscall 20: sys_are_any_keys_pressed
// Arguments: rdi = uint8_t* scancodes, rsi = uint64_t count
// Returns 1 if any key is pressed, 0 if none are pressed
uint64_t sys_are_any_keys_pressed_wrapper(uint64_t scancodes_ptr, uint64_t count)
{
    return (uint64_t)are_any_keys_pressed((uint8_t *)scancodes_ptr, count);
}

// Syscall 21: sys_are_all_keys_pressed
// Arguments: rdi = uint8_t* scancodes, rsi = uint64_t count
// Returns 1 if all keys are pressed, 0 if any key is not pressed
uint64_t sys_are_all_keys_pressed_wrapper(uint64_t scancodes_ptr, uint64_t count)
{
    return (uint64_t)are_all_keys_pressed((uint8_t *)scancodes_ptr, count);
}

// Syscall 23: sys_render_screen
// No arguments
// Swaps back buffer to front buffer (displays all drawing commands)
uint64_t sys_render_screen_wrapper(void)
{
    swap_buffers();
    return 0;
}