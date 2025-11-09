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
    "Count lines from stdin"
};
