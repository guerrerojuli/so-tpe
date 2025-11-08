#include "ctype.h"

int isAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
int isDigit(char c){
    return c >= '0' && c <= '9';
}
int isSpace(char c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}
int isLower(char c){
    return c >= 'a' && c <= 'z';
}
int isUpper(char c){
    return c >= 'A' && c <= 'Z';
}
int isAlnum(char c){
    return isAlpha(c) || isDigit(c);
}
char tolower(char c){
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    return c;
}