// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <lib.h>
#include <videoDriver.h>
#include <registers.h>
#include <semaphores.h>
#include <scheduler.h>
#include <consoleDriver.h>

// External function from scheduler to kill foreground process
extern void kill_foreground_process(void);

#define KEYBOARD_DATA_PORT 0x60 // Standard PS/2 data port
#define BUFFER_SIZE 1024        // 1 KiB circular buffer

#define SCANCODE_CAPS_LOCK 0x3A
#define SCANCODE_LSHIFT_PRESS 0x2A
#define SCANCODE_RSHIFT_PRESS 0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_LCTRL_PRESS 0x1D
#define SCANCODE_LCTRL_RELEASE 0x9D
#define PRINTABLE_SCANCODE_LIMIT 58

// Optimization: Bit masks for faster key detection
#define SCANCODE_MASK 0x7F // Lower 7 bits contain the key code
#define RELEASE_FLAG 0x80  // High bit indicates key release

// Optimized key detection using bit operations
#define IS_CAPS_LOCK(scancode) ((scancode) == SCANCODE_CAPS_LOCK)
#define IS_SHIFT_PRESS(scancode) ((scancode) == SCANCODE_LSHIFT_PRESS || (scancode) == SCANCODE_RSHIFT_PRESS)
#define IS_SHIFT_RELEASE(scancode) ((scancode) == SCANCODE_LSHIFT_RELEASE || (scancode) == SCANCODE_RSHIFT_RELEASE)
#define IS_PRINTABLE(scancode) ((scancode) < PRINTABLE_SCANCODE_LIMIT && \
                                kbd_lookup_table[(scancode)].normal != 0)

//=============================================================================
// OPTIMIZED DATA STRUCTURES
//=============================================================================

// Optimized lookup table combining normal and shift characters
typedef struct
{
    char normal;
    char shifted;
} key_mapping_t;

// Single optimized lookup table (better cache locality)
static const key_mapping_t kbd_lookup_table[128] = {
    {0, 0}, {27, 27}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {'\b', '\b'}, {'\t', '\t'}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'}, {'\n', '\n'}, {0, 0}, {'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'}, {'\'', '"'}, {'`', '~'}, {0, 0}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, {0, 0}, {'*', '*'}, {0, 0}, {' ', ' '}, // Space key
    // Rest initialized to {0, 0}
};

//=============================================================================
// OPTIMIZED STATE MANAGEMENT
//=============================================================================

// Keyboard state packed into a single structure for better cache locality
typedef struct
{
    char buffer[BUFFER_SIZE];
    uint64_t write_ptr;
    uint64_t read_ptr;
    uint64_t count;
    uint8_t caps_lock : 1;
    uint8_t shift_pressed : 1;
    uint8_t ctrl_pressed : 1;       // Track Ctrl key state
    uint8_t reserved : 4;           // Reserved for future flags
    uint32_t active_bindings_count; // Counter instead of recalculating
} __attribute__((packed)) kbd_state_t;

static kbd_state_t kbd_state = {0};

// Keyboard semaphore for blocking I/O
static sem_t kbd_semaphore = 0;  // Semaphore ID for keyboard

// Initialize keyboard driver and semaphore
void init_keyboard(void) {
    // Initialize semaphore with value 0 (no data initially)
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
    kbd_state.write_ptr = (kbd_state.write_ptr + 1) & (BUFFER_SIZE - 1); // Faster modulo for power of 2
    kbd_state.count++;

    // Signal that data is available
    sem_post(&kbd_semaphore);
}

static inline char extract_char(void)
{
    char c = kbd_state.buffer[kbd_state.read_ptr];
    kbd_state.read_ptr = (kbd_state.read_ptr + 1) & (BUFFER_SIZE - 1); // Faster modulo for power of 2
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
    uint8_t scancode = raw_scancode & SCANCODE_MASK;  // Remove release flag
    uint8_t is_release = raw_scancode & RELEASE_FLAG; // Check if it's a key release

    // Handle Ctrl key press and release
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

    // Handle shift keys for both press and release
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

    // Handle remaining keys only on key press
    if (!is_release)
    {
        if (IS_CAPS_LOCK(scancode))
        {
            kbd_state.caps_lock ^= 1; // XOR toggle (faster than !)
            return;
        }

        // Check for Ctrl key combinations
        if (kbd_state.ctrl_pressed && IS_PRINTABLE(scancode))
        {
            // Get the base character
            char base_char = kbd_lookup_table[scancode].normal;

            // Check for Ctrl+C (scancode for 'c' is 0x2E)
            if (base_char == 'c' || base_char == 'C')
            {
                kill_foreground_process();
                return;
            }

            // Check for Ctrl+D (scancode for 'd' is 0x20)
            if (base_char == 'd' || base_char == 'D')
            {
                // Insert EOF marker
                if (has_buffer_space())
                {
                    insert_char(-1);  // EOF is -1
                }
                return;
            }
        }

        // Fast path: Check if scancode is printable before lookup
        if (IS_PRINTABLE(scancode) && !kbd_state.ctrl_pressed)
        {
            char ascii_code = get_char_for_scancode(scancode);

            // Fast buffer insertion if character is valid and buffer has space
            if (ascii_code && has_buffer_space())
            {
                insert_char(ascii_code);
            }
        }
    }

    // All other scancodes are implicitly ignored (no else needed)
}

char getChar(void)
{
    return is_buffer_empty() ? 0 : extract_char();
}

// Blocking version of getChar - waits until a character is available
char getCharBlocking(void)
{
    // TP2_SO approach: No foreground check - trust the file descriptor system
    // If a process has STDIN, it can read from keyboard
    // This avoids race conditions and complex state management

    // Wait on semaphore until data is available
    sem_wait(&kbd_semaphore);

    // Once we wake up, we know there's data in the buffer
    return extract_char();
}