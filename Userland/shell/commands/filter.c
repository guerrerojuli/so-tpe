#include "stdio.h"
#include "stddef.h"
#include "ctype.h"
#include "commands.h"

static int filter_func(int argc, char **argv)
{
    int c;

    // Read from stdin until EOF
    while ((c = getchar()) != EOF)
    {
        char lower = tolower(c);
        // Check if it's a vowel (a, e, i, o, u)
        if (lower == 'a' || lower == 'e' || lower == 'i' ||
            lower == 'o' || lower == 'u')
        {
            putchar(c); // Output the original character (preserves case)
        }
    }

    // Print newline at the end
    putchar('\n');

    return 0;
}

command filter_cmd = {
    "filter",
    filter_func,
    "Filter vowels from stdin"
};
