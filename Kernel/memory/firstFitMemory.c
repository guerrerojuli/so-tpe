// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifdef FIRSTFIT

#include <stdint.h>
#include "../include/lib.h"
#include <stddef.h>

/*
 * KHEAPFLAG_USED - Flag to mark a chunk as used
 */
#define KHEAPFLAG_USED 0x80000000

/*
 * KHEAPHDRLCAB - Header for each memory chunk
 */
typedef struct _KHEAPHDRLCAB {
    uint32_t prevsize;   // Size of previous chunk
    uint32_t flagsize;   // [Used flag (1 bit)] [Size (31 bits)]
} KHEAPHDRLCAB;

/*
 * KHEAPBLOCKLCAB - Represents a contiguous memory block
*/
typedef struct _KHEAPBLOCKLCAB {
    uint32_t size;                       // Total block size
    uint32_t used;                       // Bytes allocated to users
    struct _KHEAPBLOCKLCAB *next;       // Next block in linked list
} KHEAPBLOCKLCAB;

/*
 * KHEAPLCAB - Main heap manager structure
 */
typedef struct _KHEAPLCAB {
    KHEAPBLOCKLCAB *fblock;  // First block
    uint32_t bcnt;            // Block count
} KHEAPLCAB;

// Global kernel heap instance
// LCAB = Linked Chunk Allocation Block
static KHEAPLCAB kernel_heap;

void k_heapLCABInit(KHEAPLCAB *heap) {
    heap->fblock = 0;
    heap->bcnt = 0;
}

/*
 * k_heapLCABAddBlock - Add a memory block to the heap manager
 */
int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t addr, uint32_t size) {
    KHEAPBLOCKLCAB *hb = (KHEAPBLOCKLCAB*)addr;
    KHEAPHDRLCAB *hdr;

    // Initialize the block header
    hb->size = size;
    hb->used = sizeof(KHEAPBLOCKLCAB) + sizeof(KHEAPHDRLCAB);  // Account for block header and initial chunk header

    // Add this block to the front of the linked list
    hb->next = heap->fblock;
    heap->fblock = hb;

    // Create the initial free chunk header right after the block header
    // &hb[1] points to the byte right after the block header
    hdr = (KHEAPHDRLCAB*)&hb[1];

    // Set the chunk size: total block size minus block header minus some padding
    // 32 bytes padding is for safety/alignment
    hdr->flagsize = hb->size - (sizeof(KHEAPBLOCKLCAB) + 32);

    // first chunk, so no previous chunk
    hdr->prevsize = 0;

    // Increment block count
    ++heap->bcnt;

    return 1;
}

/*
 * k_heapLCABAlloc - Allocate memory from the heap
 */
void* k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size) {
    KHEAPBLOCKLCAB *hb;
    KHEAPHDRLCAB *hdr, *_hdr;
    uint32_t sz;
    uint8_t fg;
    uint32_t checks = 0;
    uint32_t bc = 0;

    // Iterate through all blocks
    for (hb = heap->fblock; hb; hb = hb->next) {
        // Check if this block have enough free space
        if ((hb->size - hb->used) >= (size + sizeof(KHEAPHDRLCAB))) {
            ++bc;

            // Start at the first chunk in this block
            hdr = (KHEAPHDRLCAB*)&hb[1];

            // Walk through all chunks in this block
            while ((uintptr_t)hdr < ((uintptr_t)hb + hb->size)) {
                ++checks;

                // Extract the used flag (bit 31)
                fg = hdr->flagsize >> 31;

                // Extract the size (bits 0-30)
                // Mask off the flag bit using 0x7fffffff
                sz = hdr->flagsize & 0x7fffffff;

                if (!fg) { //if Free chunk
                    if (sz >= size) { // Large enough for our allocation
                        // Split if there's enough room for another chunk header + 16 bytes
                        if (sz > (size + sizeof(KHEAPHDRLCAB) + 16)) {
                            // Create a new chunk header after our allocation
                            _hdr = (KHEAPHDRLCAB*)((uintptr_t)&hdr[1] + size);

                            // The new chunk gets the remaining space
                            _hdr->flagsize = sz - (size + sizeof(KHEAPHDRLCAB));
                            _hdr->prevsize = size;  // Points back to our chunk

                            // Mark our chunk as used and set its size
                            hdr->flagsize = KHEAPFLAG_USED | size;

                            // Account for the new header overhead
                            hb->used += sizeof(KHEAPHDRLCAB);
                        } else {
                            // Don't split, just mark the whole chunk as used
                            hdr->flagsize |= KHEAPFLAG_USED;
                        }

                        // Account for the user data
                        hb->used += size;

                        // Return pointer to user data (after the header)
                        return &hdr[1];
                    }
                }

                // Move to the next chunk
                hdr = (KHEAPHDRLCAB*)((uintptr_t)&hdr[1] + sz);
            }
        }
    }

    // No suitable free chunk found
    return 0;
}

/*
 * k_heapLCABFree - Free previously allocated memory
 */
void k_heapLCABFree(KHEAPLCAB *heap, void *ptr) {
    KHEAPHDRLCAB *hdr, *phdr, *nhdr;
    KHEAPBLOCKLCAB *hb;
    uint32_t sz;

    // Find which block contains this pointer
    for (hb = heap->fblock; hb; hb = hb->next) {
        if (((uintptr_t)ptr > (uintptr_t)hb) &&
            ((uintptr_t)ptr < (uintptr_t)hb + hb->size)) {

            // Get the chunk header
            hdr = (KHEAPHDRLCAB*)((uintptr_t)ptr - sizeof(KHEAPHDRLCAB));

            // Mark this chunk as free by clearing the used flag
            hdr->flagsize &= ~KHEAPFLAG_USED;

            // Update the block's used count
            // we only subtract the data size, not the header
            uint32_t size_to_free = (hdr->flagsize & 0x7fffffff);
            if (hb->used >= size_to_free) {
                hb->used -= size_to_free;
            } else {
                hb->used = 0;  // Prevent underflow
            }

            // Try to get the previous chunk header
            if (hdr->prevsize) {
                // prevsize tells us how far back to look
                phdr = (KHEAPHDRLCAB*)((uintptr_t)hdr -
                       (sizeof(KHEAPHDRLCAB) + hdr->prevsize));
            } else {
                phdr = 0;  // This is the first chunk
            }

            // Get the next chunk header
            sz = hdr->flagsize & 0x7fffffff;
            nhdr = (KHEAPHDRLCAB*)((uintptr_t)&hdr[1] + sz);

            // Check if next header is out of bounds
            if ((uintptr_t)nhdr >= ((uintptr_t)hb + hb->size)) {
                nhdr = 0;  // No next chunk
            }

            // COALESCE WITH NEXT CHUNK
            // If the next chunk exists and is free, merge with it
            if (nhdr) {
                if (!(nhdr->flagsize & KHEAPFLAG_USED)) {
                    // our chunk absorbs the next chunk
                    // Add next chunk's size plus its header overhead
                    hdr->flagsize += sizeof(KHEAPHDRLCAB) +
                                    (nhdr->flagsize & 0x7fffffff);

                    // We eliminated a header, so reduce used count
                    if (hb->used >= sizeof(KHEAPHDRLCAB)) {
                        hb->used -= sizeof(KHEAPHDRLCAB);
                    } else {
                        hb->used = 0;  // Prevent underflow
                    }

                    // Update the chunk after next to point back to us
                    nhdr = (KHEAPHDRLCAB*)((uintptr_t)&hdr[1] +
                           (hdr->flagsize & 0x7fffffff));
                    if ((uintptr_t)nhdr < ((uintptr_t)hb + hb->size)) {
                        nhdr->prevsize = hdr->flagsize & 0x7fffffff;
                    }
                }
            }

            // If the previous chunk exists and is free, let it absorb us
            if (phdr) {
                if (!(phdr->flagsize & KHEAPFLAG_USED)) {
                    // Previous chunk absorbs us
                    // Add our size plus our header overhead
                    phdr->flagsize += sizeof(KHEAPHDRLCAB) +
                                     (hdr->flagsize & 0x7fffffff);

                    // We eliminated a header
                    if (hb->used >= sizeof(KHEAPHDRLCAB)) {
                        hb->used -= sizeof(KHEAPHDRLCAB);
                    } else {
                        hb->used = 0;  // Prevent underflow
                    }

                    // Update current chunk
                    hdr = phdr;

                    // Update the next chunk to point back to the merged chunk
                    nhdr = (KHEAPHDRLCAB*)((uintptr_t)&hdr[1] +
                           (hdr->flagsize & 0x7fffffff));
                    if ((uintptr_t)nhdr < ((uintptr_t)hb + hb->size)) {
                        nhdr->prevsize = hdr->flagsize & 0x7fffffff;
                    }
                }
            }

            return;
        }
    }
}

// Unified memory manager interface implementation
void* mm_alloc(uint32_t size) {
    return k_heapLCABAlloc(&kernel_heap, size);
}

void mm_free(void *ptr) {
    k_heapLCABFree(&kernel_heap, ptr);
}

void mm_init(uintptr_t start, uint32_t size) {
    k_heapLCABInit(&kernel_heap);
    k_heapLCABAddBlock(&kernel_heap, start, size);
}

void mm_get_stats(uint64_t *total, uint64_t *free) {
    *total = 0;
    *free = 0;

    // Iterate through all blocks in the linked list
    KHEAPBLOCKLCAB *block = kernel_heap.fblock;
    while (block != NULL) {
        *total += block->size;
        *free += (block->size - block->used);
        block = block->next;
    }
}

const char* mm_get_name(void) {
    return "First-Fit";
}

#endif // FIRSTFIT
