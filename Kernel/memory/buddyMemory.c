/*
 * Buddy Memory Manager
 * Asigna bloques en potencias de 2 y fusiona/divide automáticamente
 */

#include <stdint.h>
#include <stddef.h>
#include "../include/lib.h"
#include "../include/memoryManager.h"

#define PAGE_SIZE 4096

#define PAGE_BUDDY_FLAG 0x00000001

#define PageBuddy(page) ((page)->flags & PAGE_BUDDY_FLAG)
#define SetPageBuddy(page) ((page)->flags |= PAGE_BUDDY_FLAG)
#define ClearPageBuddy(page) ((page)->flags &= ~PAGE_BUDDY_FLAG)

// Funciones auxiliares de listas
static inline void list_init(list_node_t *list)
{
  list->next = list;
  list->prev = list;
}

static inline int list_empty(list_node_t *list)
{
  return list->next == list;
}

static inline void list_add(list_node_t *node, list_node_t *head)
{
  node->next = head->next;
  node->prev = head;
  head->next->prev = node;
  head->next = node;
}

static inline void list_del(list_node_t *node)
{
  node->prev->next = node->next;
  node->next->prev = node->prev;
  node->next = node;
  node->prev = node;
}

static inline page_t *list_first_entry(list_node_t *head)
{
  return (page_t *)((uintptr_t)head->next - offsetof(page_t, lru));
}

// Conversiones PFN <-> PAGE
static inline page_t *pfn_to_page(zone_t *zone, uint64_t pfn)
{
  return &zone->pages[pfn - zone->start_pfn];
}

static inline uint64_t page_to_pfn(page_t *page)
{
  return page->pfn;
}

// Encuentra el buddy: bloque adyacente con el que se puede fusionar
static inline uint64_t get_buddy_pfn(uint64_t pfn, int order)
{
  return pfn ^ (1UL << order);
}

static inline int is_buddy(uint64_t pfn1, uint64_t pfn2, int order)
{
  return get_buddy_pfn(pfn1, order) == pfn2;
}

void buddy_init(zone_t *zone, uint64_t start_pfn, uint64_t total_pages, page_t *pages_array)
{
  int order;
  uint64_t i;

  // spin_lock_init(&zone->lock);

  zone->start_pfn = start_pfn;
  zone->total_pages = total_pages;
  zone->free_pages = 0;
  zone->pages = pages_array;

  for (order = 0; order <= MAX_ORDER; order++)
  {
    list_init(&zone->free_area[order].free_list);
    zone->free_area[order].nr_free = 0;
  }

  for (i = 0; i < total_pages; i++)
  {
    zone->pages[i].pfn = start_pfn + i;
    zone->pages[i].flags = 0;
    zone->pages[i].order = 0;
    list_init(&zone->pages[i].lru);
  }
}

static void add_to_free_list(zone_t *zone, page_t *page, int order)
{
  free_area_t *area = &zone->free_area[order];

  list_add(&page->lru, &area->free_list);
  area->nr_free++;
  zone->free_pages += (1UL << order);

  page->order = order;
  SetPageBuddy(page);
}

static void del_from_free_list(zone_t *zone, page_t *page, int order)
{
  free_area_t *area = &zone->free_area[order];

  list_del(&page->lru);
  area->nr_free--;
  zone->free_pages -= (1UL << order);

  ClearPageBuddy(page);
}

// Libera páginas e intenta fusionar con buddies libres
void buddy_free_pages(zone_t *zone, page_t *page, int order)
{
  uint64_t pfn = page_to_pfn(page);
  uint64_t buddy_pfn;
  page_t *buddy;
  // unsigned long flags;

  // spin_lock_irqsave(&zone->lock, flags);

  // Fusionar con buddies mientras sea posible
  while (order < MAX_ORDER)
  {
    buddy_pfn = get_buddy_pfn(pfn, order);

    if (buddy_pfn < zone->start_pfn ||
        buddy_pfn >= zone->start_pfn + zone->total_pages)
      break;

    buddy = pfn_to_page(zone, buddy_pfn);

    // Buddy debe estar libre y del mismo orden
    if (!PageBuddy(buddy) || buddy->order != order)
      break;

    del_from_free_list(zone, buddy, order);

    // El bloque fusionado empieza en el pfn menor
    if (buddy_pfn < pfn)
    {
      page = buddy;
      pfn = buddy_pfn;
    }

    order++;
  }

  add_to_free_list(zone, page, order);

  // spin_unlock_irqrestore(&zone->lock, flags);
}

// Asigna páginas, divide bloques grandes si es necesario
page_t *buddy_alloc_pages(zone_t *zone, int order)
{
  // unsigned long flags;
  page_t *page = NULL;
  int current_order;

  if (order > MAX_ORDER)
    return NULL;

  // spin_lock_irqsave(&zone->lock, flags);

  // Buscar bloque del tamaño adecuado
  for (current_order = order; current_order <= MAX_ORDER; current_order++)
  {
    free_area_t *area = &zone->free_area[current_order];

    if (list_empty(&area->free_list))
      continue;

    page = list_first_entry(&area->free_list);
    del_from_free_list(zone, page, current_order);

    // Dividir si es muy grande
    while (current_order > order)
    {
      current_order--;
      page_t *buddy = pfn_to_page(zone,
                                  page_to_pfn(page) + (1UL << current_order));
      add_to_free_list(zone, buddy, current_order);
    }

    break;
  }

  // spin_unlock_irqrestore(&zone->lock, flags);
  return page;
}

// Agrega memoria al sistema dividiéndola en bloques alineados
void buddy_add_memory(zone_t *zone, uint64_t start_pfn, uint64_t nr_pages)
{
  uint64_t pfn = start_pfn;

  while (nr_pages > 0)
  {
    int order = MAX_ORDER;

    // Encontrar el bloque más grande que quepa
    while (order > 0)
    {
      uint64_t block_size = 1UL << order;
      if ((pfn & (block_size - 1)) == 0 && nr_pages >= block_size)
        break;
      order--;
    }

    page_t *page = pfn_to_page(zone, pfn);
    buddy_free_pages(zone, page, order);

    pfn += (1UL << order);
    nr_pages -= (1UL << order);
  }
}

void buddy_get_stats(zone_t *zone, uint64_t *total, uint64_t *free)
{
  if (total)
    *total = zone->total_pages;
  if (free)
    *free = zone->free_pages;
}

uint64_t buddy_get_free_blocks(zone_t *zone, int order)
{
  if (order < 0 || order > MAX_ORDER)
    return 0;
  return zone->free_area[order].nr_free;
}
