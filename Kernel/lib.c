#include <stdint.h>
#include "syscalls.h"
#include "interrupts.h"

// Stack canary global variable for stack protection
uintptr_t __stack_chk_guard = 0xDEADBEEFDEADBEEFUL;

// Stack check failure function - halts CPU on stack corruption
void __attribute__((noreturn)) __stack_chk_fail(void)
{
	// Print a message to the console
	char error_msg[] = "*** KERNEL PANIC: Stack corruption detected! System halted ***\n";
	sys_write(2, error_msg, sizeof(error_msg) - 1);
	// Halt the CPU in an infinite loop
	while (1)
	{
		// Disable interrupts and halt
		_cli();
		_hlt();
	}
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
	/*
	 * memcpy does not support overlapping buffers, so always do it
	 * forwards. (Don't change this without adjusting memmove.)
	 *
	 * For speedy copying, optimize the common case where both pointers
	 * and the length are word-aligned, and copy word-at-a-time instead
	 * of byte-at-a-time. Otherwise, copy by bytes.
	 *
	 * The alignment logic below should be portable. We rely on
	 * the compiler to be reasonably intelligent about optimizing
	 * the divides and modulos out. Fortunately, it is.
	 */
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
	/*
	 * memmove handles overlapping buffers by copying in the appropriate direction.
	 * If dest > src, copy backwards to avoid overwriting source data.
	 * If dest < src, copy forwards.
	 */
	uint8_t *d = (uint8_t *)destination;
	const uint8_t *s = (const uint8_t *)source;

	if (d == s || length == 0)
	{
		return destination;
	}

	if (d > s && d < s + length)
	{
		// Overlapping, copy backwards
		for (uint64_t i = length; i > 0; i--)
		{
			d[i - 1] = s[i - 1];
		}
	}
	else
	{
		// Non-overlapping or dest < src, copy forwards
		for (uint64_t i = 0; i < length; i++)
		{
			d[i] = s[i];
		}
	}

	return destination;
}