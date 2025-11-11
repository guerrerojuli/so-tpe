

#include <stdint.h>
#include <lib.h>
#include <videoDriver.h>
#include <registers.h>
#include <semaphores.h>
#include <scheduler.h>
#include <consoleDriver.h>

extern void kill_foreground_process(void);

#define KEYBOARD_DATA_PORT 0x60
#define BUFFER_SIZE 1024

#define SCANCODE_CAPS_LOCK 0x3A
#define SCANCODE_LSHIFT_PRESS 0x2A
#define SCANCODE_RSHIFT_PRESS 0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_LCTRL_PRESS 0x1D
#define SCANCODE_LCTRL_RELEASE 0x9D
#define PRINTABLE_SCANCODE_LIMIT 58

#define SCANCODE_MASK 0x7F
#define RELEASE_FLAG 0x80

#define IS_CAPS_LOCK(scancode) ((scancode) == SCANCODE_CAPS_LOCK)
#define IS_SHIFT_PRESS(scancode) ((scancode) == SCANCODE_LSHIFT_PRESS || (scancode) == SCANCODE_RSHIFT_PRESS)
#define IS_SHIFT_RELEASE(scancode) ((scancode) == SCANCODE_LSHIFT_RELEASE || (scancode) == SCANCODE_RSHIFT_RELEASE)
#define IS_PRINTABLE(scancode) ((scancode) < PRINTABLE_SCANCODE_LIMIT && \
                                kbd_lookup_table[(scancode)].normal != 0)

typedef struct
{
    char normal;
    char shifted;
} key_mapping_t;

static const key_mapping_t kbd_lookup_table[128] = {
    {0, 0},
    {27, 27},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {'\b', '\b'},
    {'\t', '\t'},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {'\n', '\n'},
    {0, 0},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {0, 0},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {0, 0},
    {'*', '*'},
    {0, 0},
    {' ', ' '},

};

typedef struct
{
    char buffer[BUFFER_SIZE];
    uint64_t write_ptr;
    uint64_t read_ptr;
    uint64_t count;
    uint8_t caps_lock : 1;
    uint8_t shift_pressed : 1;
    uint8_t ctrl_pressed : 1;
    uint8_t reserved : 4;
    uint32_t active_bindings_count;
} __attribute__((packed)) kbd_state_t;

static kbd_state_t kbd_state = {0};

static sem_t kbd_semaphore = 0;

void init_keyboard(void)
{

    sem_init(&kbd_semaphore, 0);
}

static inline uint8_t has_buffer_space(void)
{
    return kbd_state.count < BUFFER_SIZE;
}

static inline uint8_t is_buffer_empty(void)
{
    return kbd_state.count == 0;
}

static inline void insert_char(char c)
{
    kbd_state.buffer[kbd_state.write_ptr] = c;
    kbd_state.write_ptr = (kbd_state.write_ptr + 1) & (BUFFER_SIZE - 1);
    kbd_state.count++;

    sem_post(&kbd_semaphore);
}

static inline char extract_char(void)
{
    char c = kbd_state.buffer[kbd_state.read_ptr];
    kbd_state.read_ptr = (kbd_state.read_ptr + 1) & (BUFFER_SIZE - 1);
    kbd_state.count--;
    return c;
}

static inline char get_char_for_scancode(uint8_t scancode)
{
    const key_mapping_t *mapping = &kbd_lookup_table[scancode];
    return (kbd_state.caps_lock || kbd_state.shift_pressed) ? mapping->shifted : mapping->normal;
}

void keyboard_handler(const registers_t *registers)
{
    uint8_t raw_scancode = inb(KEYBOARD_DATA_PORT);
    uint8_t scancode = raw_scancode & SCANCODE_MASK;
    uint8_t is_release = raw_scancode & RELEASE_FLAG;

    if (scancode == SCANCODE_LCTRL_PRESS && !is_release)
    {
        kbd_state.ctrl_pressed = 1;
        return;
    }

    if (raw_scancode == SCANCODE_LCTRL_RELEASE)
    {
        kbd_state.ctrl_pressed = 0;
        return;
    }

    if (IS_SHIFT_PRESS(scancode) && !is_release)
    {
        kbd_state.shift_pressed = 1;
        return;
    }

    if (IS_SHIFT_RELEASE(raw_scancode))
    {
        kbd_state.shift_pressed = 0;
        return;
    }

    if (!is_release)
    {
        if (IS_CAPS_LOCK(scancode))
        {
            kbd_state.caps_lock ^= 1;
            return;
        }

        if (kbd_state.ctrl_pressed && IS_PRINTABLE(scancode))
        {

            char base_char = kbd_lookup_table[scancode].normal;

            if (base_char == 'c' || base_char == 'C')
            {
                kill_foreground_process();
                return;
            }

            if (base_char == 'd' || base_char == 'D')
            {

                if (has_buffer_space())
                {
                    insert_char(-1);
                }
                return;
            }
        }

        if (IS_PRINTABLE(scancode) && !kbd_state.ctrl_pressed)
        {
            char ascii_code = get_char_for_scancode(scancode);

            if (ascii_code && has_buffer_space())
            {
                insert_char(ascii_code);
            }
        }
    }
}

char getChar(void)
{
    return is_buffer_empty() ? 0 : extract_char();
}

char getCharBlocking(void)
{

    sem_wait(&kbd_semaphore);

    return extract_char();
}