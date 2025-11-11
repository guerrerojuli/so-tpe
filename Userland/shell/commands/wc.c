// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 
 
 
#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int wc_func(int argc, char **argv) {
    int c;
    int line_count = 0;

     
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            line_count++;
        }
    }

     
    void *args[1] = {&line_count};
    printf("Lines: %d\n", args);

    return 0;
}

command wc_cmd = {
    "wc",
    wc_func,
    "Count lines from stdin"
};
