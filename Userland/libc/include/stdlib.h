#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>

int  atoi(const char *str);
char *itoa(int num, char* dest);
void exit(int status);
int  rand(void);

// Memory management
void *malloc(uint32_t size);
void free(void *ptr);

#endif /* _STDLIB_H */