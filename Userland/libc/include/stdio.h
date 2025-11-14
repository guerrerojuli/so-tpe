#ifndef _STDIO_H
#define _STDIO_H

#define SCANF_BUFF_MAX_SIZE 1024

#define EOF (-1)

typedef int FILE; //-V677

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int scanf(const char *format, void **args);
int printf(const char *format, void **args);

int putchar(char c);
int puts(const char *s);

int getchar(void);
char *fgets(char *s, int size, int stream);

#endif
