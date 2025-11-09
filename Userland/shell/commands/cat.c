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
    "Print stdin exactly as received"
};
