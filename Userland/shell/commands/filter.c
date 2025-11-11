// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 
 
#include "stdio.h"
#include "stddef.h"
#include "ctype.h"
#include "commands.h"

static int filter_func(int argc, char **argv)
{
    int c;

     
    while ((c = getchar()) != EOF)
    {
        char lower = tolower(c);
         
        if (lower == 'a' || lower == 'e' || lower == 'i' ||
            lower == 'o' || lower == 'u')
        {
            putchar(c);  
        }
    }

     
    putchar('\n');

    return 0;
}

command filter_cmd = {
    "filter",
    filter_func,
    "Filter vowels from stdin"
};
