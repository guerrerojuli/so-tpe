// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifdef FIRSTFIT

/*
 * First-fit allocator with implicit free list and coalescing
 */
#include <stdint.h>
#include "../include/lib.h"
#include <stddef.h>

/*
 * KHEAPFLAG_USED - Flag to mark a chunk as used
 *
 * This flag is stored in the highest bit (bit 31) of the flagsize field.
 * When set (0x80000000), the chunk is in use.
 * When clear, the chunk is free.
 */
#define KHEAPFLAG_USED 0x80000000

/*
 * KHEAPHDRLCAB - Header for each memory chunk
 *
 * Each allocated or free chunk has this header immediately before the user data.
 *
 * prevsize: Size of the previous chunk (in bytes)
 *           This allows us to walk backwards through chunks for coalescing
 *           If 0, this is the first chunk in the block
 *
 * flagsize: Combined field containing:
 *           - Bit 31: USED flag (1 = used, 0 = free)
 *           - Bits 0-30: Size of THIS chunk in bytes (max 2GB)
 *           The size does NOT include the header itself
 */
typedef struct _KHEAPHDRLCAB {
    uint32_t prevsize;   // Size of previous chunk
    uint32_t flagsize;   // [Used flag (1 bit)] [Size (31 bits)]
} KHEAPHDRLCAB;

/*
 * KHEAPBLOCKLCAB - Represents a contiguous memory block
 *
 * The heap can consist of multiple non-contiguous blocks (like pages from the OS).
 * Each block is managed independently but linked together.
 *
 * size: Total size of this block in bytes (including the block header)
 * used: Total bytes currently allocated to users (not including headers)
 * next: Pointer to the next block in the linked list
 */
typedef struct _KHEAPBLOCKLCAB {
    uint32_t size;                       // Total block size
    uint32_t used;                       // Bytes allocated to users
    struct _KHEAPBLOCKLCAB *next;       // Next block in linked list
} KHEAPBLOCKLCAB;

/*
 * KHEAPLCAB - Main heap manager structure
 *
 * fblock: First block in the linked list of memory blocks
 * bcnt: Total number of blocks in the heap
 */
typedef struct _KHEAPLCAB {
    KHEAPBLOCKLCAB *fblock;  // First block
    uint32_t bcnt;            // Block count
} KHEAPLCAB;

// Global kernel heap instance
static KHEAPLCAB kernel_heap;

/*
 * k_heapLCABInit - Initialize the heap manager
 *
 * @heap: Pointer to the heap structure to initialize
 *
 * Sets up an empty heap with no memory blocks.
 */
void k_heapLCABInit(KHEAPLCAB *heap) {
    heap->fblock = 0;   // No blocks yet
    heap->bcnt = 0;     // Zero blocks
}

/*
 * k_heapLCABAddBlock - Add a memory block to the heap
 *
 * @heap: The heap manager
 * @addr: Starting address of the memory region
 * @size: Size of the memory region in bytes
 *
 * Returns: 1 on success
 *
 * This takes a raw memory region and sets it up as a managed block.
 * The block starts with a KHEAPBLOCKLCAB header, followed by a single
 * large free chunk.
 *
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
 *
 * @heap: The heap manager
 * @size: Number of bytes to allocate
 *
 * Returns: Pointer to allocated memory, or NULL if allocation fails
 *
 * Algorithm:
 * 1. Iterate through each block
 * 2. Within each block, iterate through chunks looking for a free one
 * 3. If a free chunk is large enough:
 *    a. If a free chunk is more than (size + header + 16 bytes) large, we split it to avoid wasting space.
 *    b. Mark the chunk as used
 *    c. Update bookkeeping
 * 4. Return pointer to the user data (after the header)
 *
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
 *
 * @heap: The heap manager
 * @ptr: Pointer to memory to free
 *
 * Algorithm:
 * 1. Find which block contains this pointer
 * 2. Get the chunk header (just before the pointer)
 * 3. Mark the chunk as free
 * 4. Try to coalesce with adjacent free chunks
 *    a. Merge with next chunk if free
 *    b. Merge with previous chunk if free
 *
 * Coalescing:
 * When we free a chunk, we check the chunks immediately before and after.
 * If they're also free, we merge them into one large chunk.
 * This reduces fragmentation and makes future allocations more likely to succeed.
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
