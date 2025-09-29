#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include <stdint.h>

//=============================================================================
// CORE KEYBOARD FUNCTIONALITY
//=============================================================================

/**
 * Main keyboard interrupt handler (HIGHLY OPTIMIZED)
 * Handles all keyboard input with minimal branching and maximum performance.
 */
void keyboard_handler();

/**
 * Retrieves a single character from the keyboard buffer.
 * OPTIMIZED: Uses fast buffer operations and minimal overhead.
 * @return Character from buffer, or 0 if buffer is empty
 */
char getChar(void);

/**
 * Gets the current number of characters in the buffer.
 * @return Number of characters available to read
 */
uint64_t getCharCount(void);

/**
 * Gets the remaining buffer space.
 * @return Number of characters that can still be buffered
 */
uint64_t getBufferSpace(void);

/**
 * High-performance batch character retrieval.
 * Reads multiple characters in a single operation for maximum efficiency.
 * @param dest Destination buffer for characters
 * @param max_chars Maximum number of characters to read
 * @return Number of characters actually read
 */
uint64_t getChars(char* dest, uint64_t max_chars);

/**
 * Clears the entire keyboard buffer.
 * Fast operation that resets all buffer pointers.
 */
void clearKeyboardBuffer(void);

//=============================================================================
// OPTIMIZED FUNCTION BINDING SYSTEM
//=============================================================================

/**
 * Binds a function to a specific scancode.
 * OPTIMIZED: Uses counter-based binding mode tracking for maximum speed.
 * @param scancode The scancode to bind to (0-127)
 * @param function_ptr Function to call when scancode is detected (NULL to unbind)
 * @return 0 on success, -1 on error
 */
int bind_function(uint8_t scancode, void (*function_ptr)(void));

/**
 * Unbinds a function from a scancode.
 * OPTIMIZED: Reuses bind_function logic for consistency and performance.
 * @param scancode The scancode to unbind
 * @return 0 on success, -1 on error
 */
int unbind_function(uint8_t scancode);

/**
 * Checks if a specific scancode has a bound function.
 * @param scancode The scancode to check
 * @return 1 if bound, 0 if not bound
 */
uint8_t is_key_bound(uint8_t scancode);

/**
 * Gets the number of currently active function bindings.
 * @return Number of active bindings
 */
uint32_t get_active_bindings_count(void);

//=============================================================================
// KEYBOARD STATE QUERIES
//=============================================================================

/**
 * Checks if Caps Lock is currently active.
 * @return 1 if Caps Lock is on, 0 if off
 */
uint8_t is_caps_lock_on(void);

/**
 * Checks if Shift is currently being pressed.
 * @return 1 if Shift is pressed, 0 if not
 */
uint8_t is_shift_pressed(void);

/**
 * Checks if binding mode is currently active.
 * @return 1 if binding mode is active, 0 if not
 */
uint8_t is_binding_mode_active(void);

//=============================================================================
// ADVANCED BUFFER OPERATIONS
//=============================================================================

/**
 * Peeks at the next character without removing it from the buffer.
 * Useful for lookahead operations without consuming the character.
 * @return Next character in buffer, or 0 if buffer is empty
 */
char peekChar(void);

/**
 * Peeks at multiple characters without removing them from the buffer.
 * @param dest Destination buffer for peeked characters
 * @param max_chars Maximum number of characters to peek
 * @return Number of characters actually peeked
 */
uint64_t peekChars(char* dest, uint64_t max_chars);

//=============================================================================
// DEBUGGING AND MONITORING
//=============================================================================

/**
 * Keyboard statistics structure for debugging and monitoring.
 */
typedef struct {
    uint64_t buffer_size;        // Total buffer size
    uint64_t current_count;      // Current characters in buffer
    uint64_t write_ptr;          // Current write position
    uint64_t read_ptr;           // Current read position
    uint32_t active_bindings;    // Number of active function bindings
    uint8_t caps_lock_state;     // Caps Lock state (0 or 1)
    uint8_t shift_state;         // Shift state (0 or 1)
    uint8_t binding_mode;        // Binding mode state (0 or 1)
} kbd_stats_t;

/**
 * Gets comprehensive keyboard driver statistics.
 * Useful for debugging, monitoring, and system diagnostics.
 * @param stats Pointer to structure to fill with statistics
 */
void get_keyboard_stats(kbd_stats_t* stats);

//=============================================================================
// MULTI-KEY SUPPORT FUNCTIONS
//=============================================================================

/**
 * Check if a specific key is currently pressed.
 * Enables real-time multi-key detection for advanced input handling.
 * @param scancode The scancode of the key to check (0-127)
 * @return 1 if key is pressed, 0 if not pressed or invalid scancode
 */
uint8_t is_key_pressed(uint8_t scancode);

/**
 * Get the state of multiple keys at once.
 * Efficient batch query for multiple key states in a single call.
 * @param scancodes Array of scancodes to check
 * @param states Output array to store key states (1 = pressed, 0 = not pressed)
 * @param count Number of keys to check
 * @return Number of keys successfully checked
 */
uint64_t get_key_states(uint8_t* scancodes, uint8_t* states, uint64_t count);

/**
 * Check if any of the specified keys are pressed.
 * Useful for checking key combinations or alternative key bindings.
 * @param scancodes Array of scancodes to check
 * @param count Number of keys to check
 * @return 1 if any key is pressed, 0 if none are pressed
 */
uint8_t are_any_keys_pressed(uint8_t* scancodes, uint64_t count);

/**
 * Check if all of the specified keys are pressed.
 * Perfect for complex key combinations and chord detection.
 * @param scancodes Array of scancodes to check
 * @param count Number of keys to check
 * @return 1 if all keys are pressed, 0 if any key is not pressed
 */
uint8_t are_all_keys_pressed(uint8_t* scancodes, uint64_t count);

#endif