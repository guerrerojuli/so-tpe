#ifndef MEMORY_MANAGER_INTERFACE_H
#define MEMORY_MANAGER_INTERFACE_H

#include <stdint.h>

// Unified interface for both memory managers
void* mm_alloc(uint32_t size);
void mm_free(void *ptr);
void mm_init(uintptr_t start, uint32_t size);
void mm_get_stats(uint64_t *total, uint64_t *free);

#endif
