#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "unistd.h"
#include "string.h"
#include "stddef.h"

int scanf(const char *format, void** args){
    int i = 0, j = 0;
    char scan_buff[SCANF_BUFF_MAX_SIZE];
    /* Read user input one character at a time until we encounter a newline
       or fill the buffer – this avoids blocking until 1 024 characters are
       typed. */
    uint32_t len = 0;
    while (len < SCANF_BUFF_MAX_SIZE - 1) {
        char c = getchar();  /* blocks until one key is pressed */
        if (c == '\n' || c == '\r') {
            putchar(c);
            break;
        }
        else if(c == '\b'){
            if(len > 0){
                len--;
                putchar(c);
            }
        }else{
            scan_buff[len++] = c;
            putchar(c);
        }
       
    }
    scan_buff[len] = 0;
    while(format[i] != 0 && scan_buff[j] != '\n' && scan_buff[j] != 0){
        if(format[i] == '%'){
            i++;
            switch(format[i]){
                case 'd':             // int
                    int *num = (int*)*args;
                    *num = atoi(scan_buff+j);
                    while(isDigit(scan_buff[j])){
                        j++;
                    }
                    args++;
                    break;
                case 's':             // string
                    char *str = (char*)*args;
                    while(scan_buff[j] != 0 && !isSpace(scan_buff[j])){
                        *str = scan_buff[j];
                        str++;
                        j++;
                    }
                *str = 0;
                args++;
                j++; 
                    break;
                case 'c':             // char
                    char *c = (char*)*args;
                    *c = scan_buff[j];
                    args++;
                    j++;
                    break;
                default:
                    return -1;
            }
        }
        else{
            return -1;
        }
        while(isSpace(scan_buff[j])){
            j++;
        }
        i++;
    }
    return j;
}

int printf(const char *format, void **args){
    int i = 0, toReturn = 0;
    char num_str[20];
    for(i = 0; format[i] != 0; i++){
        if(format[i] == '%'){
            i++;
            switch(format[i]){
                case 'd': {
                    int *num = (int*)*args;
                    toReturn += puts(itoa(*num, num_str));
                    args++;
                    break;
                }
                case 's': {
                    char *str = (char*)*args;
                    toReturn += puts(str);
                    args++;
                    break;
                }
                case 'c': {
                    char *c = (char*)*args;
                    putchar(*c);
                    toReturn++;
                    args++;
                    break;
                }
                default:
                    return -1;
            }
        }
        else{
            int start = i;
            while(format[i] != 0 && format[i] != '%'){
                i++;
            }
            int len = i - start;
            sys_write(STDOUT, &format[start], len);
            toReturn += len;
            i--; // compensate for for-loop increment
        }
    }
    return toReturn;
}

int putchar(char c){
    sys_write(STDOUT, &c, 1);
    return c;
}

int puts(const char *s){
    int len = strlen(s);
    sys_write(STDOUT, s, len);
    return len;
}

int getchar(void){
    char c;
    sys_read(STDIN, &c, 1);
    return c;
}

char *fgets(char *s, int size, FILE *stream) {
    if (s == NULL || size <= 0) {
        return NULL;
    }
    
    // For now, only support stdin (stream 0)
    if (stream != STDIN) {
        return NULL;
    }
    
    int i = 0;
    char c;
    
    // Read characters until newline, EOF, or buffer full
    while (i < size - 1) {
        if (sys_read(STDIN, &c, 1) <= 0) {
            // EOF or error
            if (i == 0) {
                return NULL;  // No characters read
            }
            break;  // Some characters were read
        }
        
        if (c == '\n' || c == '\r') {
            putchar(c);
            break;
        }
        else if(c == '\b'){
            if(i > 0){
                i--;
                putchar(c);
            }
        }else{
            s[i++] = c;
            putchar(c);
        }        
    }
    
    // Null-terminate the string
    s[i] = '\0';
    
    return s;
}