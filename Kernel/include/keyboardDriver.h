#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include <stdint.h>

void keyboard_handler();

void init_keyboard(void);

char getChar(void);

char getCharBlocking(void);

#endif