#ifndef _STDIO_H
#define _STDIO_H

// PVS-Studio: Suppress warnings for custom printf/scanf implementation
// -V111: Custom printf/scanf with void** instead of varargs
// -V576, -V618, -V111: Format string warnings for custom implementation
//-V:printf:111,576,618,719,303
//-V:scanf:111,576,618,719,303

#define SCANF_BUFF_MAX_SIZE 1024

#define EOF (-1)

// Simple FILE type for basic stream operations (bare-metal environment)
//-V:FILE:677
typedef int FILE;

// Standard file descriptors (as integer values)
#define STDIN  0
#define STDOUT 1
#define STDERR 2

int scanf(const char *format, void **args);
int printf(const char *format, void **args);

int putchar(char c);
int puts(const char *s);

int getchar(void);
char *fgets(char *s, int size, int stream);

#endif /* _STDIO_H */
