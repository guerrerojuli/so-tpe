#include "stdio.h"
#include "stddef.h"
#include "commands.h"

static int cat_func(int argc, char **argv) {
    int c;

    // Read from stdin until EOF
    while ((c = getchar()) != EOF) {
        putchar(c);
    }

    return 0;
}

command cat_cmd = {
    "cat",
    cat_func,
    "Print stdin exactly as received",
    "CAT(1)                       User Commands                       CAT(1)\n\n"
    "NAME\n"
    "       cat - concatenate and print\n\n"
    "SYNOPSIS\n"
    "       cat\n\n"
    "DESCRIPTION\n"
    "       Reads from standard input and prints each character to standard\n"
    "       output exactly as received. Acts as a passthrough filter in pipes.\n"
    "       Reading stops when EOF is received (pipe closes or Ctrl+D).\n\n"
    "EXAMPLES\n"
    "       echo hello | cat          # Passes through 'hello'\n"
    "       loop 1 | cat | wc &       # Part of a pipeline\n"
    "       cat                       # Read from keyboard until Ctrl+D\n\n"
    "NOTES\n"
    "       - Does not accept file arguments (no filesystem support)\n"
    "       - Useful as identity filter in pipelines\n"
    "       - Returns when the writer closes the pipe (EOF)\n\n"
};
