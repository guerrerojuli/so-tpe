

#include <stdint.h>
#include "syscalls.h"
#include "interrupts.h"

uintptr_t __stack_chk_guard = 0xDEADBEEFDEADBEEFUL;

void __attribute__((noreturn)) __stack_chk_fail(void)
{

	char error_msg[] = "*** KERNEL PANIC: Stack corruption detected! System halted ***\n";
	sys_write(2, (uint64_t)error_msg, sizeof(error_msg) - 1, 0, 0, 0);

	while (1)
	{

		_cli();
		_hlt();
	}
	__builtin_unreachable();
}

void *memset(void *destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char *dst = (char *)destination;

	while (length--)
		dst[length] = chr;

	return destination;
}

void *memcpy(void *destination, const void *source, uint64_t length)
{

	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
			(uint64_t)source % sizeof(uint32_t) == 0 &&
			length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *)destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t *d = (uint8_t *)destination;
		const uint8_t *s = (const uint8_t *)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

void *memmove(void *destination, const void *source, uint64_t length)
{

	uint8_t *d = (uint8_t *)destination;
	const uint8_t *s = (const uint8_t *)source;

	if (d == s || length == 0)
	{
		return destination;
	}

	if (d > s && d < s + length)
	{

		for (uint64_t i = length; i > 0; i--)
		{
			d[i - 1] = s[i - 1];
		}
	}
	else
	{

		for (uint64_t i = 0; i < length; i++)
		{
			d[i] = s[i];
		}
	}

	return destination;
}

uint64_t strlen(const char *str)
{
	uint64_t len = 0;
	while (str[len] != '\0')
	{
		len++;
	}
	return len;
}

char *strcpy(char *dest, const char *src)
{
	char *original_dest = dest;
	while ((*dest++ = *src++) != '\0')
		;
	return original_dest;
}