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
    "Filter vowels from stdin",
    "FILTER(1)                   User Commands                   FILTER(1)\n\n"
    "NAME\n"
    "       filter - filter vowels from input\n\n"
    "SYNOPSIS\n"
    "       filter\n\n"
    "DESCRIPTION\n"
    "       Reads from standard input and outputs only vowel characters.\n"
    "       Both uppercase and lowercase vowels (A, E, I, O, U, a, e, i, o, u)\n"
    "       are recognized and passed through, preserving their original case.\n"
    "       All other characters (consonants, digits, symbols, spaces) are\n"
    "       filtered out. Reading stops when EOF is received (pipe closes or\n"
    "       Ctrl+D). A newline is added at the end of the output.\n\n"
    "EXAMPLES\n"
    "       echo \"Hello World\" | filter    # Output: \"eoo\"\n"
    "       echo \"AEIOU123\" | filter       # Output: \"AEIOU\"\n"
    "       loop 1 | filter                 # Filter vowels from loop output\n\n"
    "NOTES\n"
    "       - Commonly used in pipes to extract vowels from text\n"
    "       - Preserves the case of vowels (uppercase/lowercase)\n"
    "       - Returns when the writer closes the pipe (EOF)\n\n"};
