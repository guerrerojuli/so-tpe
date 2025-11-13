// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <consoleDriver.h>
#include <videoDriver.h>
#include <interrupts.h>
#include <lib.h>
#include <stdbool.h>
#include <stddef.h>

#define TAB_SIZE 4
#define SPACE_CHAR ' '
#define NULL_CHAR '\0'
#define NEWLINE_CHAR '\n'
#define TAB_CHAR '\t'
#define BACKSPACE_CHAR '\b'

#define DEFAULT_TEXT_COLOR 0xFFFFFF
#define DEFAULT_BACKGROUND_COLOR 0x000000

#define TAB_MASK (~3)

#define CONSOLE_BUFFER_SIZE (CONSOLE_WIDTH * CONSOLE_HEIGHT * 2)
#define MAX_VISIBLE_CHARS (CONSOLE_WIDTH * CONSOLE_HEIGHT)

#define IS_VALID_CONSOLE_X(x) ((x) < CONSOLE_WIDTH)
#define IS_VALID_CONSOLE_Y(y) ((y) < CONSOLE_HEIGHT)
#define IS_VALID_CONSOLE_POS(x, y) (IS_VALID_CONSOLE_X(x) && IS_VALID_CONSOLE_Y(y))
#define IS_VALID_BUFFER_POS(pos) ((pos) < buffer_length && (pos) >= 0)

typedef struct
{
    uint64_t screen_x;
    uint64_t screen_y;
    uint64_t current_line;
} render_position_t;

static uint64_t console_calculate_visible_start(void);
static void console_add_char(char c, uint64_t color);
static void console_backspace(void);
static void console_render_from_position(uint64_t start_pos);
static void console_render_range(uint64_t start_pos, uint64_t end_pos);
static void console_render_incremental(uint64_t from_pos);
static void console_render_all(void);
static void console_scroll_fast(uint64_t old_visible_start, uint64_t new_visible_start);
static inline bool should_render_char(char c);

static console_char_t console_buffer[CONSOLE_BUFFER_SIZE];
static uint64_t buffer_length = 0;
static uint64_t cursor_position = 0;
static uint64_t last_visible_start = 0;

static void count_lines_to_position(uint64_t target_pos, uint64_t *line_count, uint64_t *final_x)
{
    uint64_t current_line = 0;
    uint64_t current_x = 0;
    uint64_t chars_per_line = get_chars_per_line();

    for (uint64_t pos = 0; pos <= target_pos && pos < buffer_length; pos++)
    {
        char c = console_buffer[pos].c;

        if (c == NEWLINE_CHAR)
        {
            current_line++;
            current_x = 0;
        }
        else if (c == TAB_CHAR)
        {
            current_x = (current_x + TAB_SIZE) & TAB_MASK;
            if (current_x >= chars_per_line)
            {
                current_line++;
                current_x = 0;
            }
        }
        else
        {
            current_x++;
            if (current_x >= chars_per_line)
            {
                current_line++;
                current_x = 0;
            }
        }
    }

    *line_count = current_line;
    *final_x = current_x;
}

static uint64_t find_buffer_position_for_line(uint64_t target_line)
{
    uint64_t line_count = 0;
    uint64_t pos = 0;
    uint64_t current_x = 0;
    uint64_t chars_per_line = get_chars_per_line();

    while (pos < buffer_length && line_count < target_line)
    {
        char c = console_buffer[pos].c;

        if (c == NEWLINE_CHAR)
        {
            line_count++;
            current_x = 0;
        }
        else if (c == TAB_CHAR)
        {
            current_x = (current_x + TAB_SIZE) & TAB_MASK;
            if (current_x >= chars_per_line)
            {
                line_count++;
                current_x = 0;
            }
        }
        else
        {
            current_x++;
            if (current_x >= chars_per_line)
            {
                line_count++;
                current_x = 0;
            }
        }
        pos++;
    }

    return pos;
}

static uint64_t console_calculate_visible_start(void)
{
    uint64_t lines_per_screen = get_screen_height_pixels() / get_font_height();

    uint64_t cursor_line, cursor_x;
    count_lines_to_position(cursor_position, &cursor_line, &cursor_x);

    if (cursor_line >= lines_per_screen)
    {
        uint64_t target_line = cursor_line - lines_per_screen + 1;
        return find_buffer_position_for_line(target_line);
    }

    return 0;
}

static inline bool should_render_char(char c)
{
    return (c != SPACE_CHAR && c != NULL_CHAR && c != NEWLINE_CHAR && c != TAB_CHAR);
}

static void console_render_from_position(uint64_t start_pos)
{
    console_render_range(start_pos, buffer_length);
}

static void console_render_range(uint64_t start_pos, uint64_t end_pos)
{

    uint64_t font_width = get_font_width();
    uint64_t font_height = get_font_height();
    uint64_t screen_width = get_screen_width_pixels();
    uint64_t lines_per_screen = get_screen_height_pixels() / font_height;

    render_position_t pos;
    uint64_t visible_start = console_calculate_visible_start();

    pos.screen_x = 0;
    pos.screen_y = 0;
    pos.current_line = 0;

    for (uint64_t p = visible_start; p < start_pos && p < buffer_length && pos.current_line < lines_per_screen; p++)
    {
        char c = console_buffer[p].c;

        if (c == NEWLINE_CHAR)
        {
            pos.current_line++;
            pos.screen_x = 0;
            pos.screen_y = pos.current_line * font_height;
        }
        else if (c == TAB_CHAR)
        {
            uint64_t new_x = (pos.screen_x / font_width + TAB_SIZE) & TAB_MASK;
            pos.screen_x = new_x * font_width;

            if (pos.screen_x >= screen_width)
            {
                pos.current_line++;
                pos.screen_x = 0;
                pos.screen_y = pos.current_line * font_height;
            }
        }
        else
        {
            pos.screen_x += font_width;

            if (pos.screen_x >= screen_width)
            {
                pos.current_line++;
                pos.screen_x = 0;
                pos.screen_y = pos.current_line * font_height;
            }
        }
    }

    for (uint64_t buffer_pos = start_pos;
         buffer_pos < end_pos && buffer_pos < buffer_length && pos.current_line < lines_per_screen;
         buffer_pos++)
    {

        char c = console_buffer[buffer_pos].c;
        uint64_t color = console_buffer[buffer_pos].color;

        if (c == NEWLINE_CHAR)
        {

            if (pos.screen_x < screen_width)
            {
                draw_rect(DEFAULT_BACKGROUND_COLOR, pos.screen_x, pos.screen_y,
                          screen_width - pos.screen_x, font_height);
            }
        }
        else if (c == TAB_CHAR)
        {

            draw_rect(DEFAULT_BACKGROUND_COLOR, pos.screen_x, pos.screen_y,
                      font_width, font_height);
        }
        else
        {

            draw_rect(DEFAULT_BACKGROUND_COLOR, pos.screen_x, pos.screen_y,
                      font_width, font_height);

            if (should_render_char(c))
            {
                draw_char(c, color, pos.screen_x, pos.screen_y);
            }
        }

        if (c == NEWLINE_CHAR)
        {
            pos.current_line++;
            pos.screen_x = 0;
            pos.screen_y = pos.current_line * font_height;
        }
        else if (c == TAB_CHAR)
        {
            uint64_t new_x = (pos.screen_x / font_width + TAB_SIZE) & TAB_MASK;
            pos.screen_x = new_x * font_width;

            if (pos.screen_x >= screen_width)
            {
                pos.current_line++;
                pos.screen_x = 0;
                pos.screen_y = pos.current_line * font_height;
            }
        }
        else
        {
            pos.screen_x += font_width;

            if (pos.screen_x >= screen_width)
            {
                pos.current_line++;
                pos.screen_x = 0;
                pos.screen_y = pos.current_line * font_height;
            }
        }
    }
}

static void console_render_incremental(uint64_t from_pos)
{
    if (from_pos >= buffer_length)
    {
        return;
    }

    //_cli();

    console_render_range(from_pos, buffer_length);
    swap_buffers();

    //_sti();
}

static void console_render_all(void)
{
    uint64_t start_pos = console_calculate_visible_start();

    //_cli();

    clear_screen(DEFAULT_BACKGROUND_COLOR);
    console_render_from_position(start_pos);
    swap_buffers();

    //_sti();
}

static void console_scroll_fast(uint64_t old_visible_start, uint64_t new_visible_start)
{
    if (old_visible_start == new_visible_start)
    {
        return;
    }

    uint64_t font_height = get_font_height();
    uint64_t screen_width = get_screen_width_pixels();
    uint64_t screen_height = get_screen_height_pixels();

    uint64_t old_line = 0, new_line = 0, temp_x;
    count_lines_to_position(old_visible_start, &old_line, &temp_x);
    count_lines_to_position(new_visible_start, &new_line, &temp_x);

    uint64_t lines_scrolled = (new_line > old_line) ? (new_line - old_line) : 0;

    if (lines_scrolled == 0 || lines_scrolled > 10)
    {
        console_render_all();
        return;
    }

    uint64_t scroll_pixels = lines_scrolled * font_height;

    if (scroll_pixels >= screen_height / 2)
    {
        console_render_all();
        return;
    }

    //_cli();

    uint8_t *back_buffer = get_back_buffer();
    uint64_t bytes_per_pixel = 3;
    uint64_t pitch = screen_width * bytes_per_pixel;

    uint64_t remaining_height = screen_height - scroll_pixels;
    if (remaining_height > 0)
    {
        memmove(back_buffer,
                back_buffer + (scroll_pixels * pitch),
                remaining_height * pitch);
    }

    uint8_t bg_blue = DEFAULT_BACKGROUND_COLOR & 0xFF;
    uint8_t bg_green = (DEFAULT_BACKGROUND_COLOR >> 8) & 0xFF;
    uint8_t bg_red = (DEFAULT_BACKGROUND_COLOR >> 16) & 0xFF;

    for (uint64_t y = remaining_height; y < screen_height; y++)
    {
        for (uint64_t x = 0; x < screen_width; x++)
        {
            uint64_t offset = (x * bytes_per_pixel) + (y * pitch);
            back_buffer[offset] = bg_blue;
            back_buffer[offset + 1] = bg_green;
            back_buffer[offset + 2] = bg_red;
        }
    }

    //_sti();

    console_render_from_position(new_visible_start);

    swap_buffers();
}

static void handle_buffer_overflow(void)
{
    uint64_t shift_amount = CONSOLE_BUFFER_SIZE / 4;
    memmove(&console_buffer[0], &console_buffer[shift_amount],
            (buffer_length - shift_amount) * sizeof(console_char_t));
    buffer_length -= shift_amount;
    cursor_position -= shift_amount;
}

static void insert_char_at_cursor(char c, uint64_t color)
{

    if (cursor_position < buffer_length)
    {

        memmove(&console_buffer[cursor_position + 1], &console_buffer[cursor_position],
                (buffer_length - cursor_position) * sizeof(console_char_t));
    }

    console_buffer[cursor_position].c = c;
    console_buffer[cursor_position].color = color;
    buffer_length++;
    cursor_position++;
}

static void console_add_char(char c, uint64_t color)
{
    if (buffer_length >= CONSOLE_BUFFER_SIZE - 1)
    {
        handle_buffer_overflow();
    }

    insert_char_at_cursor(c, color);
}

static void console_backspace(void)
{
    if (cursor_position == 0)
    {
        return;
    }

    cursor_position--;

    if (cursor_position < buffer_length - 1)
    {
        memmove(&console_buffer[cursor_position], &console_buffer[cursor_position + 1],
                (buffer_length - cursor_position - 1) * sizeof(console_char_t));
    }
    buffer_length--;
}

void console_write(const char *data, uint64_t data_len, uint64_t color)
{
    if (data == NULL || data_len == 0)
    {
        return;
    }

    uint64_t start_cursor = cursor_position;
    uint64_t start_visible = console_calculate_visible_start();

    for (uint64_t i = 0; i < data_len; i++)
    {
        if (data[i] == BACKSPACE_CHAR)
        {
            console_backspace();

            console_render_all();
            last_visible_start = console_calculate_visible_start();
            return;
        }
        else
        {
            console_add_char(data[i], color);
        }
    }

    uint64_t new_visible_start = console_calculate_visible_start();
    if (new_visible_start != start_visible)
    {

        console_scroll_fast(last_visible_start, new_visible_start);
        last_visible_start = new_visible_start;
    }
    else
    {

        console_render_incremental(start_cursor);
    }
}

void console_clear(void)
{
    buffer_length = 0;
    cursor_position = 0;
    last_visible_start = 0;

    clear_screen(DEFAULT_BACKGROUND_COLOR);
    swap_buffers();
}
