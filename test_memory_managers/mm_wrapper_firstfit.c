#include <stdint.h>
#include <stddef.h>

// Declare globals before including
typedef struct _KHEAPLCAB {
    struct _KHEAPBLOCKLCAB *fblock;
    uint32_t bcnt;
} KHEAPLCAB;

typedef struct _KHEAPBLOCKLCAB {
    uint32_t size;
    uint32_t used;
    struct _KHEAPBLOCKLCAB *next;
} KHEAPBLOCKLCAB;

KHEAPLCAB kernel_heap;

// Include the implementation
#include "../Kernel/memory/firstFitMemory.c"
