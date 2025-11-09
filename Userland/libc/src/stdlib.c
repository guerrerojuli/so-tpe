#include "stdlib.h"
#include "ctype.h"
#include "unistd.h"
#include "stdint.h"
#include "stddef.h"

void exit(int status)
{
    uint64_t pid = sys_get_pid();
    sys_kill_process(pid, (uint64_t)status);
    // Should never reach here
    while (1)
        ;
}

int atoi(const char *str)
{
    int num = 0;
    while (isDigit(*str))
    {
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}
char *itoa(int num, char *dest)
{
    if (num == 0)
    {
        dest[0] = '0';
        dest[1] = 0;
        return dest;
    }

    int i = 0;
    int isNeg = 0;
    if (num < 0)
    {
        isNeg = 1;
        num = -num;
    }

    while (num > 0)
    {
        dest[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (isNeg)
    {
        dest[i++] = '-';
    }

    dest[i] = 0;

    // reverse the string in-place
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char tmp = dest[start];
        dest[start] = dest[end];
        dest[end] = tmp;
        start++;
        end--;
    }

    return dest;
}

int rand(void)
{
    uint64_t pid = sys_get_pid();
    uint64_t ticks = sys_get_ticks();
    uint32_t rand_state = (uint32_t)((pid * 1103515245) + ticks);

    rand_state = rand_state * 1103515245 + 12345;
    return (int)((rand_state / 65536) % 32768);
}

void *malloc(uint32_t size)
{
    return (void *)sys_malloc((uint64_t)size);
}

void free(void *ptr)
{
    if (ptr != NULL) {
        sys_free((uint64_t)ptr);
    }
}