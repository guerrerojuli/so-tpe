#ifndef _STDIO_H
#define _STDIO_H

#define SCANF_BUFF_MAX_SIZE 1024

// Simple FILE type for basic stream operations
typedef int FILE;

int scanf(const char *format, void **args);
int printf(const char *format, void **args);

int putchar(char c);
int puts(const char *s);

int getchar(void);
char *fgets(char *s, int size, FILE *stream);

#endif /* _STDIO_H */
