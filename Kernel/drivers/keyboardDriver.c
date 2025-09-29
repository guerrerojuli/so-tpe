#include <stdint.h>
#include <lib.h>
#include <videoDriver.h>
#include <registers.h>

#define KEYBOARD_DATA_PORT 0x60                 // Standard PS/2 data port
#define BUFFER_SIZE 1024                        // 1 KiB circular buffer
#define MAX_BINDINGS 128

// Well-known scancode values
#define SCANCODE_CAPS_LOCK 0x3A
#define SCANCODE_LSHIFT_PRESS 0x2A
#define SCANCODE_RSHIFT_PRESS 0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_ESC 0x01
#define PRINTABLE_SCANCODE_LIMIT 58

// Optimization: Bit masks for faster key detection
#define SCANCODE_MASK 0x7F            // Lower 7 bits contain the key code
#define RELEASE_FLAG 0x80            // High bit indicates key release

// Multi-key support: Array to track key states (1 = pressed, 0 = released)
static uint8_t key_states[128] = {0}; // Support for 128 scancodes

// Optimized key detection using bit operations
#define IS_CAPS_LOCK(scancode) ((scancode) == SCANCODE_CAPS_LOCK)
#define IS_SHIFT_PRESS(scancode) ((scancode) == SCANCODE_LSHIFT_PRESS || (scancode) == SCANCODE_RSHIFT_PRESS)
#define IS_SHIFT_RELEASE(scancode) ((scancode) == SCANCODE_LSHIFT_RELEASE || (scancode) == SCANCODE_RSHIFT_RELEASE)
#define IS_ESC_PRESS(scancode) ((scancode) == SCANCODE_ESC)
#define IS_PRINTABLE(scancode) ((scancode) < PRINTABLE_SCANCODE_LIMIT && \
                                 kbd_lookup_table[(scancode)].normal != 0)

//=============================================================================
// OPTIMIZED DATA STRUCTURES
//=============================================================================

// Optimized lookup table combining normal and shift characters
typedef struct {
    char normal;
    char shifted;
} key_mapping_t;

// Single optimized lookup table (better cache locality)
static const key_mapping_t kbd_lookup_table[128] = {
    {0, 0}, {27, 27}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {'\b', '\b'}, {'\t', '\t'},
    {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'},
    {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'}, {'\n', '\n'}, {0, 0}, {'a', 'A'}, {'s', 'S'},
    {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'},
    {'\'', '"'}, {'`', '~'}, {0, 0}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'},
    {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, {0, 0}, {'*', '*'},
    {0, 0}, {' ', ' '}, // Space key
    // Rest initialized to {0, 0}
};

//=============================================================================
// OPTIMIZED STATE MANAGEMENT
//=============================================================================

// Keyboard state packed into a single structure for better cache locality
typedef struct {
    char buffer[BUFFER_SIZE];
    uint64_t write_ptr;
    uint64_t read_ptr;
    uint64_t count;
    uint8_t caps_lock : 1;
    uint8_t shift_pressed : 1;
    uint8_t binding_mode_active : 1;
    uint8_t reserved : 5; // Reserved for future flags
    uint32_t active_bindings_count; // Counter instead of recalculating
} __attribute__((packed)) kbd_state_t;

static kbd_state_t kbd_state = {0};

// Optimized function binding table with better memory layout
static void (*function_bindings[MAX_BINDINGS])(void) = {0};

//=============================================================================
// PERFORMANCE OPTIMIZATION FUNCTIONS
//=============================================================================

/**
 * Fast inline buffer space check
 */
static inline uint8_t has_buffer_space(void) {
    return kbd_state.count < BUFFER_SIZE;
}

/**
 * Fast inline buffer empty check
 */
static inline uint8_t is_buffer_empty(void) {
    return kbd_state.count == 0;
}

/**
 * Optimized character insertion with minimal branching
 */
static inline void insert_char_fast(char c) {
    kbd_state.buffer[kbd_state.write_ptr] = c;
    kbd_state.write_ptr = (kbd_state.write_ptr + 1) & (BUFFER_SIZE - 1); // Faster modulo for power of 2
    kbd_state.count++;
}

/**
 * Optimized character extraction with minimal branching
 */
static inline char extract_char_fast(void) {
    char c = kbd_state.buffer[kbd_state.read_ptr];
    kbd_state.read_ptr = (kbd_state.read_ptr + 1) & (BUFFER_SIZE - 1); // Faster modulo for power of 2
    kbd_state.count--;
    return c;
}

/**
 * Fast character lookup with single table access
 */
static inline char get_char_for_scancode(uint8_t scancode) {
    const key_mapping_t* mapping = &kbd_lookup_table[scancode];
    return (kbd_state.caps_lock || kbd_state.shift_pressed) ? mapping->shifted : mapping->normal;
}

//=============================================================================
// HIGHLY OPTIMIZED KEYBOARD HANDLER
//=============================================================================

/**
 * Main keyboard interrupt handler (HIGHLY OPTIMIZED + Multi-key support)
 * Optimizations:
 * - Minimal branching using early returns
 * - Single table lookup
 * - Cached state access
 * - Fast bit operations
 * - Multi-key state tracking
 */
void keyboard_handler(const registers_t * registers) {
    uint8_t raw_scancode = inb(KEYBOARD_DATA_PORT);
    uint8_t scancode = raw_scancode & SCANCODE_MASK;  // Remove release flag
    uint8_t is_release = raw_scancode & RELEASE_FLAG; // Check if it's a key release
    
    // Multi-key support: Update key state array
    if (scancode < 128) {
        key_states[scancode] = is_release ? 0 : 1;
    }
    
    // Fast path: Check for function bindings first (only on key press, not release)
    if (!is_release && scancode < MAX_BINDINGS && function_bindings[scancode] != 0) {
        function_bindings[scancode]();
        return;
    }
    
    // Handle special keys with optimized bit checks (only on key press)
    // These should ALWAYS be processed, regardless of binding mode
    if (!is_release) {
        if (IS_ESC_PRESS(scancode)) {
            save_registers(registers);
            return;
        }
    }
    
    // Fast path: If in binding mode and no function bound, early exit
    // BUT only for non-special keys (special keys handled above)
    if (kbd_state.binding_mode_active && function_bindings[scancode] == 0) {
        return;
    }
    
    // Handle remaining keys only on key press
    if (!is_release) {
        if (IS_CAPS_LOCK(scancode)) {
            kbd_state.caps_lock ^= 1; // XOR toggle (faster than !)
            return;
        }
        
        // Fast path: Check if scancode is printable before lookup
        if (IS_PRINTABLE(scancode)) {
            char ascii_code = get_char_for_scancode(scancode);
            
            // Fast buffer insertion if character is valid and buffer has space
            if (ascii_code && has_buffer_space()) {
                insert_char_fast(ascii_code);
            }
        }
    }
    
    // Handle shift keys for both press and release
    if (IS_SHIFT_PRESS(scancode)) {
        kbd_state.shift_pressed = 1;
        return;
    }
    
    if (IS_SHIFT_RELEASE(raw_scancode)) {
        kbd_state.shift_pressed = 0;
        return;
    }
    
    // All other scancodes are implicitly ignored (no else needed)
}

//=============================================================================
// OPTIMIZED PUBLIC API FUNCTIONS
//=============================================================================

/**
 * Optimized character retrieval with minimal overhead
 */
char getChar(void) {
    return is_buffer_empty() ? 0 : extract_char_fast();
}

/**
 * Fast buffer status check
 */
uint64_t getCharCount(void) {
    return kbd_state.count;
}

/**
 * Fast buffer space check
 */
uint64_t getBufferSpace(void) {
    return BUFFER_SIZE - kbd_state.count;
}

/**
 * High-performance batch character retrieval
 */
uint64_t getChars(char* dest, uint64_t max_chars) {
    if (!dest || max_chars == 0) return 0;
    
    uint64_t chars_read = 0;
    uint64_t to_read = (max_chars < kbd_state.count) ? max_chars : kbd_state.count;
    
    // Optimized batch read with minimal function call overhead
    for (uint64_t i = 0; i < to_read; i++) {
        dest[chars_read++] = extract_char_fast();
    }
    
    return chars_read;
}

/**
 * Optimized buffer clearing
 */
void clearKeyboardBuffer(void) {
    kbd_state.write_ptr = 0;
    kbd_state.read_ptr = 0;
    kbd_state.count = 0;
}

//=============================================================================
// OPTIMIZED FUNCTION BINDING SYSTEM
//=============================================================================

/**
 * Optimized function binding with counter-based tracking
 */
int bind_function(uint8_t scancode, void (*function_ptr)(void)) {
    if (scancode >= MAX_BINDINGS) {
        return -1;
    }
    
    // Check if this is a new binding or replacement
    uint8_t was_bound = (function_bindings[scancode] != 0);
    
    function_bindings[scancode] = function_ptr;
    
    // Update binding counter efficiently
    if (function_ptr != 0) {
        if (!was_bound) {
            kbd_state.active_bindings_count++;
        }
        kbd_state.binding_mode_active = 1;
    } else {
        if (was_bound) {
            kbd_state.active_bindings_count--;
        }
        kbd_state.binding_mode_active = (kbd_state.active_bindings_count > 0);
    }
    
    return 0;
}

/**
 * Optimized function unbinding
 */
int unbind_function(uint8_t scancode) {
    return bind_function(scancode, 0); // Reuse optimized bind_function
}

/**
 * Fast binding status check
 */
uint8_t is_key_bound(uint8_t scancode) {
    return (scancode < MAX_BINDINGS) && (function_bindings[scancode] != 0);
}

/**
 * Get active bindings count
 */
uint32_t get_active_bindings_count(void) {
    return kbd_state.active_bindings_count;
}

//=============================================================================
// KEYBOARD STATE QUERY FUNCTIONS
//=============================================================================

/**
 * Fast modifier key state queries
 */
uint8_t is_caps_lock_on(void) {
    return kbd_state.caps_lock;
}

uint8_t is_shift_pressed(void) {
    return kbd_state.shift_pressed;
}

uint8_t is_binding_mode_active(void) {
    return kbd_state.binding_mode_active;
}

//=============================================================================
// ADVANCED FEATURES
//=============================================================================

/**
 * Peek at next character without removing it from buffer
 */
char peekChar(void) {
    return is_buffer_empty() ? 0 : kbd_state.buffer[kbd_state.read_ptr];
}

/**
 * Peek at multiple characters
 */
uint64_t peekChars(char* dest, uint64_t max_chars) {
    if (!dest || max_chars == 0 || is_buffer_empty()) return 0;
    
    uint64_t chars_to_peek = (max_chars < kbd_state.count) ? max_chars : kbd_state.count;
    uint64_t read_pos = kbd_state.read_ptr;
    
    for (uint64_t i = 0; i < chars_to_peek; i++) {
        dest[i] = kbd_state.buffer[read_pos];
        read_pos = (read_pos + 1) & (BUFFER_SIZE - 1);
    }
    
    return chars_to_peek;
}

/**
 * Get keyboard statistics for debugging/monitoring
 */
typedef struct {
    uint64_t buffer_size;
    uint64_t current_count;
    uint64_t write_ptr;
    uint64_t read_ptr;
    uint32_t active_bindings;
    uint8_t caps_lock_state;
    uint8_t shift_state;
    uint8_t binding_mode;
} kbd_stats_t;

void get_keyboard_stats(kbd_stats_t* stats) {
    if (!stats) return;
    
    stats->buffer_size = BUFFER_SIZE;
    stats->current_count = kbd_state.count;
    stats->write_ptr = kbd_state.write_ptr;
    stats->read_ptr = kbd_state.read_ptr;
    stats->active_bindings = kbd_state.active_bindings_count;
    stats->caps_lock_state = kbd_state.caps_lock;
    stats->shift_state = kbd_state.shift_pressed;
    stats->binding_mode = kbd_state.binding_mode_active;
}

//=============================================================================
// MULTI-KEY SUPPORT FUNCTIONS
//=============================================================================

/**
 * Check if a specific key is currently pressed
 * @param scancode The scancode of the key to check
 * @return 1 if key is pressed, 0 if not pressed or invalid scancode
 */
uint8_t is_key_pressed(uint8_t scancode) {
    if (scancode >= 128) return 0;
    return key_states[scancode];
}

/**
 * Get the state of multiple keys at once
 * @param scancodes Array of scancodes to check
 * @param states Output array to store key states (1 = pressed, 0 = not pressed)
 * @param count Number of keys to check
 * @return Number of keys successfully checked
 */
uint64_t get_key_states(uint8_t* scancodes, uint8_t* states, uint64_t count) {
    if (!scancodes || !states || count == 0) return 0;
    
    uint64_t checked = 0;
    for (uint64_t i = 0; i < count; i++) {
        if (scancodes[i] < 128) {
            states[i] = key_states[scancodes[i]];
            checked++;
        } else {
            states[i] = 0; // Invalid scancode
        }
    }
    
    return checked;
}

/**
 * Check if any of the specified keys are pressed
 * @param scancodes Array of scancodes to check
 * @param count Number of keys to check
 * @return 1 if any key is pressed, 0 if none are pressed
 */
uint8_t are_any_keys_pressed(uint8_t* scancodes, uint64_t count) {
    if (!scancodes || count == 0) return 0;
    
    for (uint64_t i = 0; i < count; i++) {
        if (scancodes[i] < 128 && key_states[scancodes[i]]) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Check if all of the specified keys are pressed
 * @param scancodes Array of scancodes to check
 * @param count Number of keys to check
 * @return 1 if all keys are pressed, 0 if any key is not pressed
 */
uint8_t are_all_keys_pressed(uint8_t* scancodes, uint64_t count) {
    if (!scancodes || count == 0) return 0;
    
    for (uint64_t i = 0; i < count; i++) {
        if (scancodes[i] >= 128 || !key_states[scancodes[i]]) {
            return 0;
        }
    }
    
    return 1;
}