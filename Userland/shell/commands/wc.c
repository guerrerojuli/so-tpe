#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int wc_func(int argc, char **argv) {
    char buffer[256];
    int n;
    int line_count = 0;

    while ((n = sys_read(STDIN, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
    }

    // Print the line count
    void *args[] = {&line_count};
    printf("%d lines\n", args);

    return 0;
}

command wc_cmd = {
    "wc",
    wc_func,
    "Count lines in input",
    "Usage: wc\n"
    "Counts the number of lines in standard input.\n"
    "Press Ctrl+D to send EOF and see the count.\n"
};