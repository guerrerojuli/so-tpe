#include "commands.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"

static int cat_func(int argc, char **argv) {
    char buffer[256];
    int n;

    while ((n = sys_read(STDIN, buffer, sizeof(buffer))) > 0) {
        sys_write(STDOUT, buffer, n);
    }

    return 0;
}

command cat_cmd = {
    "cat",
    cat_func,
    "Copy stdin to stdout",
    "Usage: cat\n"
    "Reads from standard input and writes to standard output.\n"
    "Press Ctrl+D to send EOF and exit.\n"
};