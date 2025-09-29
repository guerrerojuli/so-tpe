#include "stdlib.h"
#include "ctype.h"

int atoi(const char *str){
    int num = 0;
    while(isDigit(*str)){
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}
char *itoa(int num, char* dest){
    if(num == 0){
        dest[0] = '0';
        dest[1] = 0;
        return dest;
    }

    int i = 0;
    int isNeg = 0;
    if(num < 0){
        isNeg = 1;
        num = -num;
    }

    while(num > 0){
        dest[i++] = (num % 10) + '0';
        num /= 10;
    }

    if(isNeg){
        dest[i++] = '-';
    }

    dest[i] = 0;

    // reverse the string in-place
    int start = 0;
    int end = i - 1;
    while(start < end){
        char tmp = dest[start];
        dest[start] = dest[end];
        dest[end] = tmp;
        start++;
        end--;
    }

    return dest;
}