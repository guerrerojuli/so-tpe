#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int is_vowel(char c) {
    return (c == 'a' || c == 'A' ||
            c == 'e' || c == 'E' ||
            c == 'i' || c == 'I' ||
            c == 'o' || c == 'O' ||
            c == 'u' || c == 'U');
}

static int filter_func(int argc, char **argv) {
    char buffer[256];
    char output[256];
    int n;

    while ((n = sys_read(STDIN, buffer, sizeof(buffer))) > 0) {
        int j = 0;
        for (int i = 0; i < n; i++) {
            if (!is_vowel(buffer[i])) {
                output[j++] = buffer[i];
            }
        }
        if (j > 0) {
            sys_write(STDOUT, output, j);
        }
    }

    return 0;
}

command filter_cmd = {
    "filter",
    filter_func,
    "Filter vowels from input",
    "Usage: filter\n"
    "Removes all vowels (a,e,i,o,u) from standard input.\n"
    "Press Ctrl+D to send EOF and exit.\n"
};