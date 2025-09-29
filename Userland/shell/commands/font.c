#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "unistd.h"
#include "commands.h"

#define FONT_SMALL 1   // Smallest font size supported
#define FONT_MEDIUM 2  // Medium font size
#define FONT_LARGE 3   // Largest font size

static int font_func(void) {
    if (arg_count < 2) {
        printf("Usage: font <size>\n", NULL);
        printf("Available sizes: small, medium, large\n", NULL);
        return -1;
    }
    
    char *size = current_args[1];
    if (strcmp(size, "small") == 0) {
        sys_set_font_size(FONT_SMALL);
        printf("Font set to small\n", NULL);
    } else if (strcmp(size, "medium") == 0) {
        sys_set_font_size(FONT_MEDIUM);
        printf("Font set to medium\n", NULL);
    } else if (strcmp(size, "large") == 0) {
        sys_set_font_size(FONT_LARGE);
        printf("Font set to large\n", NULL);
    } else {
        printf("Unknown font size: %s\n", (void**)&size);
        return -1;
    }
    return 0;
}

command font_cmd = {
    "font", 
    font_func, 
    "Changes font size",
    "FONT(1)                     User Commands                     FONT(1)\n\n"
    "NAME\n"
    "       font - change font size\n\n"
    "SYNOPSIS\n"
    "       font <size>\n\n"
    "DESCRIPTION\n"
    "       Change the display font size.\n\n"
    "OPTIONS\n"
    "       small    Set font to small size\n"
    "       medium   Set font to medium size\n"
    "       large    Set font to large size\n\n"
    "EXAMPLES\n"
    "       font small\n"
    "       font large\n\n"
}; 