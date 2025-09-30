#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

// Entry-Based Memory Manager structures
typedef struct _KHEAPHDRLCAB {
    uint32_t prevsize;
    uint32_t flagsize;
} KHEAPHDRLCAB;

typedef struct _KHEAPBLOCKLCAB {
    uint32_t size;
    uint32_t used;
    struct _KHEAPBLOCKLCAB *next;
    uint32_t lastdsize;
    KHEAPHDRLCAB *lastdhdr;
} KHEAPBLOCKLCAB;

typedef struct _KHEAPLCAB {
    KHEAPBLOCKLCAB *fblock;
    uint32_t bcnt;
} KHEAPLCAB;

// Entry-Based Memory Manager functions
void k_heapLCABInit(KHEAPLCAB *heap);
int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t addr, uint32_t size);
void* k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size);
void k_heapLCABFree(KHEAPLCAB *heap, void *ptr);

#endif // MEMORY_MANAGER_H
