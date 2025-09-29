#include "commands.h"
#include "rtc.h"

static int time_func(void) {
    printDate();
    return 0;
}

command time_cmd = {
    "time", 
    time_func, 
    "Shows current time",
    "TIME(1)                     User Commands                     TIME(1)\n\n"
    "NAME\n"
    "       time - display current time\n\n"
    "SYNOPSIS\n"
    "       time\n\n"
    "DESCRIPTION\n"
    "       Display the current system time.\n"
    "       Note: Time functionality is not yet implemented in the kernel.\n\n"
}; 