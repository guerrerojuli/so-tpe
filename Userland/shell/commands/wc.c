#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int wc_func(int argc, char **argv) {
    int c;
    int line_count = 0;

    // Read from stdin until EOF
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            line_count++;
        }
    }

    // Print result
    void *args[1] = {&line_count};
    printf("Lines: %d\n", args);

    return 0;
}

command wc_cmd = {
    "wc",
    wc_func,
    "Count lines from stdin",
    "WC(1)                       User Commands                       WC(1)\n\n"
    "NAME\n"
    "       wc - word count (line counter)\n\n"
    "SYNOPSIS\n"
    "       wc\n\n"
    "DESCRIPTION\n"
    "       Reads from standard input and counts the number of lines.\n"
    "       A line is counted when a newline character (\\n) is encountered.\n"
    "       Reading stops when EOF is received (pipe closes or Ctrl+D).\n\n"
    "EXAMPLES\n"
    "       echo hello | wc           # Counts lines from echo output\n"
    "       loop 1 | wc &             # Count loop output in background\n\n"
    "NOTES\n"
    "       - Commonly used in pipes to count output lines\n"
    "       - Returns when the writer closes the pipe (EOF)\n\n"
};
