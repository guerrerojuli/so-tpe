// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifdef FIRSTFIT

#include <stdint.h>
#include "../include/lib.h"
#include <stddef.h>

// Memory block structure for the free list
typedef union MemBlock MemBlock;

union MemBlock {
    struct {
        MemBlock *next;      // Next block in free list
        uint32_t blockSize;  // Size in units
    } metadata;
    uint64_t align;  // Force alignment
};

// Static variables for memory management
static MemBlock sentinel;        // Sentinel node for circular list
static MemBlock *freeList = NULL; // Current position in free list
static uint32_t totalMemory = 0;  // Total memory in bytes
static uint32_t freeMemory = 0;   // Free memory in bytes

#define BLOCK_SIZE sizeof(MemBlock)
#define MIN_BLOCK_UNITS 2  // Minimum allocation size

// Convert bytes to block units (round up)
static inline uint32_t bytesToUnits(uint32_t bytes) {
    return (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// Convert units to bytes
static inline uint32_t unitsToBytes(uint32_t units) {
    return units * BLOCK_SIZE;
}

void mm_init(uintptr_t start, uint32_t size) {
    // Calculate how many units we can fit
    uint32_t totalUnits = size / BLOCK_SIZE;
    if (totalUnits < MIN_BLOCK_UNITS) {
        return; // Not enough memory
    }

    // Initialize the first free block
    MemBlock *initialBlock = (MemBlock *)start;
    initialBlock->metadata.blockSize = totalUnits;

    // Set up total and free memory
    totalMemory = unitsToBytes(totalUnits);
    freeMemory = totalMemory;

    // Initialize sentinel node
    sentinel.metadata.next = initialBlock;
    sentinel.metadata.blockSize = 0;

    // Make it circular
    initialBlock->metadata.next = &sentinel;

    // Set current free list pointer
    freeList = &sentinel;
}

void *mm_alloc(uint32_t size) {
    if (size == 0) {
        return NULL;
    }

    // Calculate units needed (including header)
    uint32_t unitsNeeded = bytesToUnits(size) + 1; // +1 for header
    if (unitsNeeded < MIN_BLOCK_UNITS) {
        unitsNeeded = MIN_BLOCK_UNITS;
    }

    // Search for a suitable block using first-fit
    MemBlock *current, *previous;
    previous = freeList;

    // Traverse the circular list
    for (current = previous->metadata.next;;
         previous = current, current = current->metadata.next) {

        // Found a block that's large enough
        if (current->metadata.blockSize >= unitsNeeded) {
            MemBlock *allocatedBlock;

            if (current->metadata.blockSize == unitsNeeded) {
                // Exact fit - remove entire block from free list
                previous->metadata.next = current->metadata.next;
                allocatedBlock = current;
            } else {
                // Split the block
                current->metadata.blockSize -= unitsNeeded;
                allocatedBlock = current + current->metadata.blockSize;
                allocatedBlock->metadata.blockSize = unitsNeeded;
            }

            // Update free list pointer for next search
            freeList = previous;

            // Update free memory counter
            freeMemory -= unitsToBytes(unitsNeeded);

            // Return pointer to usable memory (skip header)
            return (void *)(allocatedBlock + 1);
        }

        // We've searched the entire list
        if (current == freeList) {
            return NULL; // No suitable block found
        }
    }
}

void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    // Get block header
    MemBlock *blockToFree = ((MemBlock *)ptr) - 1;

    // Update free memory counter
    freeMemory += unitsToBytes(blockToFree->metadata.blockSize);

    // Find where to insert in the free list
    MemBlock *current;
    for (current = freeList;
         !(blockToFree > current && blockToFree < current->metadata.next);
         current = current->metadata.next) {

        // Handle wrap-around case
        if (current >= current->metadata.next &&
            (blockToFree > current || blockToFree < current->metadata.next)) {
            break;
        }
    }

    // Try to coalesce with next block
    if (blockToFree + blockToFree->metadata.blockSize == current->metadata.next) {
        // Merge with next block
        blockToFree->metadata.blockSize += current->metadata.next->metadata.blockSize;
        blockToFree->metadata.next = current->metadata.next->metadata.next;
    } else {
        // Just link to next block
        blockToFree->metadata.next = current->metadata.next;
    }

    // Try to coalesce with previous block
    if (current + current->metadata.blockSize == blockToFree) {
        // Merge with previous block
        current->metadata.blockSize += blockToFree->metadata.blockSize;
        current->metadata.next = blockToFree->metadata.next;
    } else {
        // Just link from previous block
        current->metadata.next = blockToFree;
        current = blockToFree;
    }

    // Update free list pointer
    freeList = current;
}

void mm_get_stats(uint64_t *total, uint64_t *free) {
    *total = (uint64_t)totalMemory;
    *free = (uint64_t)freeMemory;
}

const char *mm_get_name(void) {
    return "First-Fit";
}

#endif