#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

// Default to FIRSTFIT if neither is defined
#if !defined(FIRSTFIT) && !defined(BUDDY)
#define FIRSTFIT
#endif

#include "memoryManagerInterface.h"

// Entry-Based Memory Manager structures
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

// Entry-Based Memory Manager functions
void k_heapLCABInit(KHEAPLCAB *heap);
int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t addr, uint32_t size);
void *k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size);
void k_heapLCABFree(KHEAPLCAB *heap, void *ptr);

// ============================================
// Buddy Memory Manager
// ============================================

#define MAX_ORDER 10

// Nodo de lista doblemente enlazada
typedef struct list_node
{
    struct list_node *next;
    struct list_node *prev;
} list_node_t;

// Estructura de página
typedef struct page
{
    list_node_t lru;
    uint32_t order;
    uint32_t flags;
    uint64_t pfn;
} page_t;

// Lista de bloques libres para cada orden
typedef struct free_area
{
    list_node_t free_list;
    uint64_t nr_free;
} free_area_t;

// Zona de memoria
typedef struct zone
{
    free_area_t free_area[MAX_ORDER + 1];
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t start_pfn;
    page_t *pages;
} zone_t;

// Buddy Memory Manager functions
void buddy_init(zone_t *zone, uint64_t start_pfn, uint64_t total_pages, page_t *pages_array);
void buddy_free_pages(zone_t *zone, page_t *page, int order);
page_t *buddy_alloc_pages(zone_t *zone, int order);
void buddy_add_memory(zone_t *zone, uint64_t start_pfn, uint64_t nr_pages);
void buddy_get_stats(zone_t *zone, uint64_t *total, uint64_t *free);
uint64_t buddy_get_free_blocks(zone_t *zone, int order);

// Conditional externs based on selected memory manager
#ifdef FIRSTFIT
extern KHEAPLCAB kernel_heap;
#endif

#ifdef BUDDY
extern zone_t buddy_zone;
extern page_t buddy_pages[];
#endif

#endif // MEMORY_MANAGER_H
