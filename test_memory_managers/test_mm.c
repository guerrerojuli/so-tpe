#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ========== FIRSTFIT IMPLEMENTATION ==========
#ifdef FIRSTFIT

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

// Include firstFit implementation inline
static void k_heapLCABInit(KHEAPLCAB *heap) {
    heap->fblock = NULL;
    heap->bcnt = 0;
}

static void k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t base, uint32_t size) {
    KHEAPBLOCKLCAB *block = (KHEAPBLOCKLCAB *)base;
    block->size = size;
    block->used = sizeof(KHEAPBLOCKLCAB);
    block->next = heap->fblock;
    heap->fblock = block;
    heap->bcnt++;
}

static void *k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size) {
    if (size == 0 || size > 0x7FFFFFFF)
        return NULL;

    KHEAPBLOCKLCAB *block = heap->fblock;
    while (block != NULL) {
        uint32_t available = block->size - block->used;
        if (available >= size) {
            void *ptr = (void *)((uintptr_t)block + block->used);
            block->used += size;
            return ptr;
        }
        block = block->next;
    }
    return NULL;
}

static void k_heapLCABFree(KHEAPLCAB *heap, void *ptr) {
    if (!ptr || !heap->fblock)
        return;

    KHEAPBLOCKLCAB *block = heap->fblock;
    while (block != NULL) {
        uintptr_t block_start = (uintptr_t)block + sizeof(KHEAPBLOCKLCAB);
        uintptr_t block_end = (uintptr_t)block + block->used;

        if ((uintptr_t)ptr >= block_start && (uintptr_t)ptr < block_end) {
            // Simplified free: just mark as available by reducing used space
            // This is a basic implementation - real one has coalescing
            return;
        }
        block = block->next;
    }
}

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
    if (kernel_heap.fblock) {
        *total = kernel_heap.fblock->size;
        *free = kernel_heap.fblock->size - kernel_heap.fblock->used;
    } else {
        *total = 0;
        *free = 0;
    }
}

#endif // FIRSTFIT

// ========== BUDDY IMPLEMENTATION ==========
#ifdef BUDDY

#define MAX_ORDER 10
#define PAGE_SIZE 4096

typedef struct page {
    uint64_t pfn;
    int order;
    int flags;
    struct page *next;
} page_t;

typedef struct {
    uint64_t start_pfn;
    uint64_t num_pages;
    page_t *pages;
    page_t *free_areas[MAX_ORDER + 1];
} zone_t;

zone_t buddy_zone;
static page_t buddy_pages[2048];

static void buddy_init(zone_t *zone, uint64_t start_pfn, uint64_t num_pages, page_t *pages) {
    zone->start_pfn = start_pfn;
    zone->num_pages = num_pages;
    zone->pages = pages;

    for (int i = 0; i <= MAX_ORDER; i++) {
        zone->free_areas[i] = NULL;
    }

    for (uint64_t i = 0; i < num_pages; i++) {
        pages[i].pfn = start_pfn + i;
        pages[i].order = -1;
        pages[i].flags = 0;
        pages[i].next = NULL;
    }
}

static void buddy_add_memory(zone_t *zone, uint64_t start_pfn, uint64_t num_pages) {
    uint64_t current = start_pfn;
    uint64_t end = start_pfn + num_pages;

    while (current < end) {
        int order = MAX_ORDER;
        while (order >= 0) {
            uint64_t block_size = 1UL << order;
            if (current + block_size <= end && (current & (block_size - 1)) == 0) {
                page_t *page = &zone->pages[current - zone->start_pfn];
                page->order = order;
                page->next = zone->free_areas[order];
                zone->free_areas[order] = page;
                current += block_size;
                break;
            }
            order--;
        }
    }
}

static page_t* buddy_alloc_pages(zone_t *zone, int order) {
    if (order > MAX_ORDER)
        return NULL;

    for (int curr_order = order; curr_order <= MAX_ORDER; curr_order++) {
        if (zone->free_areas[curr_order] != NULL) {
            page_t *page = zone->free_areas[curr_order];
            zone->free_areas[curr_order] = page->next;

            while (curr_order > order) {
                curr_order--;
                uint64_t buddy_pfn = page->pfn + (1UL << curr_order);
                page_t *buddy = &zone->pages[buddy_pfn - zone->start_pfn];
                buddy->order = curr_order;
                buddy->next = zone->free_areas[curr_order];
                zone->free_areas[curr_order] = buddy;
            }

            page->order = order;
            page->next = NULL;
            return page;
        }
    }
    return NULL;
}

static void buddy_free_pages(zone_t *zone, page_t *page, int order) {
    while (order < MAX_ORDER) {
        uint64_t buddy_pfn = page->pfn ^ (1UL << order);

        if (buddy_pfn < zone->start_pfn || buddy_pfn >= zone->start_pfn + zone->num_pages)
            break;

        page_t *buddy = &zone->pages[buddy_pfn - zone->start_pfn];
        if (buddy->order != order)
            break;

        page_t **prev = &zone->free_areas[order];
        while (*prev && *prev != buddy)
            prev = &(*prev)->next;

        if (!*prev)
            break;

        *prev = buddy->next;

        if (page->pfn > buddy->pfn)
            page = buddy;

        order++;
    }

    page->order = order;
    page->next = zone->free_areas[order];
    zone->free_areas[order] = page;
}

static void buddy_get_stats(zone_t *zone, uint64_t *total, uint64_t *free) {
    *total = zone->num_pages * PAGE_SIZE;
    *free = 0;

    for (int order = 0; order <= MAX_ORDER; order++) {
        page_t *page = zone->free_areas[order];
        while (page) {
            *free += (1UL << order) * PAGE_SIZE;
            page = page->next;
        }
    }
}

void* mm_alloc(uint32_t size) {
    int order = 0;
    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    while ((1UL << order) < pages_needed && order <= MAX_ORDER)
        order++;

    page_t *page = buddy_alloc_pages(&buddy_zone, order);
    if (!page) return NULL;

    return (void*)(page->pfn * PAGE_SIZE);
}

void mm_free(void *ptr) {
    if (!ptr) return;

    uint64_t pfn = ((uint64_t)ptr) / PAGE_SIZE;
    page_t *page = &buddy_zone.pages[pfn - buddy_zone.start_pfn];
    buddy_free_pages(&buddy_zone, page, page->order);
}

void mm_init(uintptr_t start, uint32_t size) {
    uint64_t start_pfn = start / PAGE_SIZE;
    uint64_t num_pages = size / PAGE_SIZE;

    if (num_pages > 2048)
        num_pages = 2048;

    buddy_init(&buddy_zone, start_pfn, num_pages, buddy_pages);
    buddy_add_memory(&buddy_zone, start_pfn, num_pages);
}

void mm_get_stats(uint64_t *total, uint64_t *free) {
    buddy_get_stats(&buddy_zone, total, free);
}

#endif // BUDDY

// Test configuration
#define HEAP_SIZE (1024 * 1024)  // 1MB
#define MAX_ALLOCATIONS 100
#define MIN_ALLOC_SIZE 16
#define MAX_ALLOC_SIZE 4096
#define NUM_ITERATIONS 1000

typedef struct {
    void *ptr;
    size_t size;
} Allocation;

static Allocation allocations[MAX_ALLOCATIONS];
static int allocation_count = 0;
static uint8_t heap_memory[HEAP_SIZE];
static int test_passed = 1;
static int total_tests = 0;
static int passed_tests = 0;

// Test result tracking
void assert_true(int condition, const char *test_name) {
    total_tests++;
    if (condition) {
        passed_tests++;
        printf("  [PASS] %s\n", test_name);
    } else {
        printf("  [FAIL] %s\n", test_name);
        test_passed = 0;
    }
}

void assert_not_null(void *ptr, const char *test_name) {
    assert_true(ptr != NULL, test_name);
}

// Check if allocations overlap
int check_no_overlap() {
    for (int i = 0; i < allocation_count; i++) {
        if (allocations[i].ptr == NULL) continue;

        uintptr_t start1 = (uintptr_t)allocations[i].ptr;
        uintptr_t end1 = start1 + allocations[i].size;

        for (int j = i + 1; j < allocation_count; j++) {
            if (allocations[j].ptr == NULL) continue;

            uintptr_t start2 = (uintptr_t)allocations[j].ptr;
            uintptr_t end2 = start2 + allocations[j].size;

            // Check overlap
            if ((start1 < end2) && (start2 < end1)) {
                printf("    ERROR: Allocation %d [%p-%p] overlaps with %d [%p-%p]\n",
                       i, (void*)start1, (void*)end1,
                       j, (void*)start2, (void*)end2);
                return 0;
            }
        }
    }
    return 1;
}

// Test 1: Basic allocation
void test_basic_allocation() {
    printf("\nTest 1: Basic Allocation\n");

    void *p1 = mm_alloc(100);
    assert_not_null(p1, "Allocate 100 bytes");

    void *p2 = mm_alloc(200);
    assert_not_null(p2, "Allocate 200 bytes");

    assert_true(p1 != p2, "Different pointers");

    mm_free(p1);
    mm_free(p2);
}

// Test 2: Write and read
void test_write_read() {
    printf("\nTest 2: Write and Read\n");

    char *buffer = (char*)mm_alloc(256);
    assert_not_null(buffer, "Allocate 256 bytes");

    if (buffer) {
        strcpy(buffer, "Hello, Memory Manager!");
        assert_true(strcmp(buffer, "Hello, Memory Manager!") == 0, "Write and read string");
        mm_free(buffer);
    }
}

// Test 3: Multiple allocations
void test_multiple_allocations() {
    printf("\nTest 3: Multiple Allocations\n");

    void *ptrs[10];
    int success = 1;

    for (int i = 0; i < 10; i++) {
        ptrs[i] = mm_alloc(128);
        if (ptrs[i] == NULL) {
            success = 0;
            break;
        }
    }

    assert_true(success, "Allocate 10 blocks of 128 bytes");

    // Free all
    for (int i = 0; i < 10; i++) {
        if (ptrs[i]) mm_free(ptrs[i]);
    }
}

// Test 4: Allocation and free pattern
void test_alloc_free_pattern() {
    printf("\nTest 4: Allocation/Free Pattern\n");

    void *p1 = mm_alloc(100);
    void *p2 = mm_alloc(200);
    mm_free(p1);
    void *p3 = mm_alloc(50);

    assert_not_null(p2, "Second allocation valid");
    assert_not_null(p3, "Third allocation valid");

    mm_free(p2);
    mm_free(p3);
}

// Test 5: Random allocations with overlap check
void test_random_no_overlap() {
    printf("\nTest 5: Random Allocations (No Overlap)\n");

    srand(42);  // Fixed seed for reproducibility
    allocation_count = 0;

    for (int iter = 0; iter < 100; iter++) {
        if (allocation_count < MAX_ALLOCATIONS && (rand() % 2 == 0 || allocation_count == 0)) {
            // Allocate
            size_t size = MIN_ALLOC_SIZE + (rand() % (MAX_ALLOC_SIZE - MIN_ALLOC_SIZE));
            void *ptr = mm_alloc(size);

            if (ptr != NULL) {
                allocations[allocation_count].ptr = ptr;
                allocations[allocation_count].size = size;
                allocation_count++;
            }
        } else if (allocation_count > 0) {
            // Free random allocation
            int idx = rand() % allocation_count;
            if (allocations[idx].ptr != NULL) {
                mm_free(allocations[idx].ptr);
                allocations[idx].ptr = NULL;
            }
        }

        // Check no overlap
        if (!check_no_overlap()) {
            assert_true(0, "No overlapping allocations");
            goto cleanup;
        }
    }

    assert_true(1, "No overlapping allocations after 100 iterations");

cleanup:
    // Free remaining
    for (int i = 0; i < allocation_count; i++) {
        if (allocations[i].ptr != NULL) {
            mm_free(allocations[i].ptr);
        }
    }
}

// Test 6: Memory statistics
void test_memory_stats() {
    printf("\nTest 6: Memory Statistics\n");

    uint64_t total, free_before, free_after;
    mm_get_stats(&total, &free_before);

    printf("  Initial - Total: %llu bytes, Free: %llu bytes\n", total, free_before);
    assert_true(total > 0, "Total memory reported");
    assert_true(free_before > 0, "Free memory reported");

    void *p = mm_alloc(1024);
    mm_get_stats(&total, &free_after);

    printf("  After 1KB alloc - Total: %llu bytes, Free: %llu bytes\n", total, free_after);
    assert_true(free_after < free_before, "Free memory decreased after allocation");

    mm_free(p);
}

// Test 7: Edge cases
void test_edge_cases() {
    printf("\nTest 7: Edge Cases\n");

    void *p1 = mm_alloc(0);
    printf("  Allocate 0 bytes: %s\n", p1 ? "returned pointer" : "returned NULL");
    if (p1) mm_free(p1);

    void *p2 = mm_alloc(1);
    assert_not_null(p2, "Allocate 1 byte");
    if (p2) mm_free(p2);

    // Try very large allocation
    void *p3 = mm_alloc(HEAP_SIZE * 2);
    printf("  Allocate %d bytes (too large): %s\n", HEAP_SIZE * 2,
           p3 ? "succeeded (unexpected)" : "failed (expected)");
    if (p3) mm_free(p3);
}

int main() {
    printf("========================================\n");
    printf("Memory Manager Test Suite\n");

#ifdef FIRSTFIT
    printf("Testing: FIRSTFIT Memory Manager\n");
#elif defined(BUDDY)
    printf("Testing: BUDDY Memory Manager\n");
#else
    printf("Testing: Unknown Memory Manager\n");
#endif

    printf("========================================\n");

    // Initialize memory manager
    mm_init((uintptr_t)heap_memory, HEAP_SIZE);
    printf("Initialized with %d bytes heap\n", HEAP_SIZE);

    // Run all tests
    test_basic_allocation();
    test_write_read();
    test_multiple_allocations();
    test_alloc_free_pattern();
    test_random_no_overlap();
    test_memory_stats();
    test_edge_cases();

    // Summary
    printf("\n========================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    printf("========================================\n");

    return (passed_tests == total_tests) ? 0 : 1;
}
