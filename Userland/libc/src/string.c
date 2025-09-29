#include "string.h"
#include "stddef.h"

int strlen(const char* s){
    int i = 0;
    while (s[i] != 0){
        i++;
    }
    return i;
}

int strcmp(const char* s1, const char* s2){
    int i;
    for (i=0;s1[i] != 0 && s2[i] != 0 && s1[i] == s2[i];i++);
    return s1[i] - s2[i];
}


void strcpy(char* dest, const char* src){
    while (*src != 0){
        *dest = *src;
        dest++;
        src++;
    }
    *dest = 0;
}

char *strcat(char *destination, const char *source){
    int i;
    for (i=0; destination[i] != 0; i++);
    for (int j=0; source[j] != 0; j++, i++){
        destination[i] = source[j];
    }
    destination[i] = 0;
    return destination;
}

char *strtok(char *str, const char *delims){
    static char *last = NULL;
    
    // If str is not NULL, start tokenizing a new string
    if (str != NULL) {
        last = str;
    } else if (last == NULL) {
        // No string to continue tokenizing
        return NULL;
    }
    
    // Skip leading delimiters
    while (*last != 0) {
        int is_delim = 0;
        for (int i = 0; delims[i] != 0; i++) {
            if (*last == delims[i]) {
                is_delim = 1;
                break;
            }
        }
        if (!is_delim) break;
        last++;
    }
    
    // If we reached end of string, no more tokens
    if (*last == 0) {
        last = NULL;
        return NULL;
    }
    
    // Mark start of current token
    char *token_start = last;
    
    // Find end of current token
    while (*last != 0) {
        int is_delim = 0;
        for (int i = 0; delims[i] != 0; i++) {
            if (*last == delims[i]) {
                is_delim = 1;
                break;
            }
        }
        if (is_delim) {
            *last = 0;  // Replace delimiter with null terminator
            last++;
            return token_start;
        }
        last++;
    }
    
    // Reached end of string, this is the last token
    last = NULL;
    return token_start;
}
