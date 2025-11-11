#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

#if !defined(FIRSTFIT) && !defined(BUDDY)
#define FIRSTFIT
#endif

#include "memoryManagerInterface.h"

typedef struct _KHEAPHDRLCAB
{
    uint32_t prevsize;
    uint32_t flagsize;
} KHEAPHDRLCAB;

typedef struct _KHEAPBLOCKLCAB
{
    uint32_t size;
    uint32_t used;
    struct _KHEAPBLOCKLCAB *next;
    uint32_t lastdsize;
    KHEAPHDRLCAB *lastdhdr;
} KHEAPBLOCKLCAB;

typedef struct _KHEAPLCAB
{
    KHEAPBLOCKLCAB *fblock;
    uint32_t bcnt;
} KHEAPLCAB;

void k_heapLCABInit(KHEAPLCAB *heap);
int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t addr, uint32_t size);
void *k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size);
void k_heapLCABFree(KHEAPLCAB *heap, void *ptr);

#define MAX_ORDER 10

typedef struct list_node
{
    struct list_node *next;
    struct list_node *prev;
} list_node_t;

typedef struct page
{
    list_node_t free_list_node;
    uint32_t order;
    uint32_t flags;
    uint64_t frame_number;
} page_t;

typedef struct free_area
{
    list_node_t free_list_head;
    uint64_t free_block_count;
} free_area_t;

typedef struct zone
{
    free_area_t free_lists[MAX_ORDER + 1];
    uint64_t total_pages;
    page_t *pages;
    uintptr_t heap_base;
} zone_t;

void buddy_free_pages(page_t *page, int order);
page_t *buddy_alloc_pages(int order);
void buddy_add_memory(uint64_t nr_pages);

#ifdef FIRSTFIT
extern KHEAPLCAB kernel_heap;
#endif

#endif
