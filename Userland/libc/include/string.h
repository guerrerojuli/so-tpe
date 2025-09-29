#ifndef _STRING_H
#define _STRING_H

int strlen(const char* s);

int strcmp(const char* s1, const char* s2);

void strcpy(char* dest, const char* src);

char *strcat(char *destination, const char *source);

char *strtok(char *str, const char *delims);

#endif /* _STRING_H */

