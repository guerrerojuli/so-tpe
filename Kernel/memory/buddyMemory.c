// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifdef BUDDY

#include <stdint.h>
#include <stddef.h>
#include "../include/lib.h"
#include "../include/memoryManager.h"

#define PAGE_SIZE 4096

static page_t buddy_pages[2048];
static zone_t buddy_zone;

#define PAGE_FREE_FLAG 0x00000001

#define PageIsFree(page) ((page)->flags & PAGE_FREE_FLAG)
#define MarkPageFree(page) ((page)->flags |= PAGE_FREE_FLAG)
#define MarkPageUsed(page) ((page)->flags &= ~PAGE_FREE_FLAG)

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
  return (page_t *)((uintptr_t)head->next - offsetof(page_t, free_list_node));
}

static inline page_t *frame_to_page(zone_t *zone, uint64_t frame_number)
{
  uint64_t base_frame = zone->heap_base / PAGE_SIZE;
  return &zone->pages[frame_number - base_frame];
}

static inline uint64_t page_to_frame_number(page_t *page)
{
  return page->frame_number;
}

static inline uint64_t get_buddy_frame_number(uint64_t frame_number, int order)
{
  return frame_number ^ (1UL << order);
}

static void add_to_free_list(zone_t *zone, page_t *page, int order)
{
  free_area_t *area = &zone->free_lists[order];

  list_add(&page[0].free_list_node, &area->free_list_head);
  area->free_block_count++;

  uint64_t nr_pages = 1UL << order;
  for (uint64_t i = 0; i < nr_pages; i++)
  {
    page[i].order = order;
    MarkPageFree(&page[i]);
  }
}

static void del_from_free_list(zone_t *zone, page_t *page, int order)
{
  free_area_t *area = &zone->free_lists[order];

  list_del(&page->free_list_node);
  area->free_block_count--;

  uint64_t nr_pages = 1UL << order;
  for (uint64_t i = 0; i < nr_pages; i++)
  {
    MarkPageUsed(&page[i]);
  }
}

void buddy_free_pages(page_t *page, int order)
{
  zone_t *zone = &buddy_zone;
  uint64_t frame_number = page_to_frame_number(page);
  uint64_t buddy_frame_number;
  page_t *buddy;
  uint64_t base_frame = zone->heap_base / PAGE_SIZE;

  while (order < MAX_ORDER)
  {
    buddy_frame_number = get_buddy_frame_number(frame_number, order);

    if (buddy_frame_number < base_frame ||
        buddy_frame_number >= base_frame + zone->total_pages)
      break;

    buddy = frame_to_page(zone, buddy_frame_number);

    if (!PageIsFree(buddy) || buddy->order != order)
      break;

    del_from_free_list(zone, buddy, order);

    if (buddy_frame_number < frame_number)
    {
      page = buddy;
      frame_number = buddy_frame_number;
    }

    order++;
  }

  add_to_free_list(zone, page, order);
}

page_t *buddy_alloc_pages(int order)
{
  zone_t *zone = &buddy_zone;

  page_t *page = NULL;
  int current_order;

  if (order > MAX_ORDER)
    return NULL;

  for (current_order = order; current_order <= MAX_ORDER; current_order++)
  {
    free_area_t *area = &zone->free_lists[current_order];

    if (list_empty(&area->free_list_head))
      continue;

    page = list_first_entry(&area->free_list_head);
    del_from_free_list(zone, page, current_order);

    while (current_order > order)
    {
      current_order--;
      page_t *buddy = frame_to_page(zone,
                                    page_to_frame_number(page) + (1UL << current_order));
      add_to_free_list(zone, buddy, current_order);
    }

    page->order = order;

    break;
  }

  return page;
}

void buddy_add_memory(uint64_t nr_pages)
{
  zone_t *zone = &buddy_zone;
  uint64_t base_frame = zone->heap_base / PAGE_SIZE;
  uint64_t frame_number = base_frame;

  while (nr_pages > 0)
  {
    int order = MAX_ORDER;

    while (order > 0)
    {
      uint64_t block_size = 1UL << order;
      if ((frame_number & (block_size - 1)) == 0 && nr_pages >= block_size)
        break;
      order--;
    }

    page_t *page = frame_to_page(zone, frame_number);
    buddy_free_pages(page, order);

    frame_number += (1UL << order);
    nr_pages -= (1UL << order);
  }
}

void *mm_alloc(uint32_t size)
{

  if (size == 0)
    return NULL;

  int order = 0;
  uint32_t pages_needed = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;

  while ((1UL << order) < pages_needed && order <= MAX_ORDER)
    order++;

  if (order > MAX_ORDER)
    return NULL;

  page_t *page = buddy_alloc_pages(order);
  if (!page)
    return NULL;

  uint64_t base_frame = buddy_zone.heap_base / PAGE_SIZE;
  uint64_t page_offset = page->frame_number - base_frame;
  return (void *)(buddy_zone.heap_base + (page_offset * PAGE_SIZE));
}

void mm_free(void *ptr)
{
  if (!ptr)
    return;

  if ((uint64_t)ptr < buddy_zone.heap_base ||
      (uint64_t)ptr >= buddy_zone.heap_base + (buddy_zone.total_pages * PAGE_SIZE))
    return;

  uint64_t offset = (uint64_t)ptr - buddy_zone.heap_base;
  uint64_t page_index = offset / PAGE_SIZE;

  if (page_index >= buddy_zone.total_pages)
    return;

  page_t *page = &buddy_zone.pages[page_index];

  if (PageIsFree(page))
    return;

  buddy_free_pages(page, page->order);
}

void mm_init(uintptr_t start, uint32_t size)
{
  uint64_t num_pages = size / PAGE_SIZE;

  if (num_pages > 2048)
    num_pages = 2048;

  buddy_zone.heap_base = start;
  buddy_zone.total_pages = num_pages;
  buddy_zone.pages = buddy_pages;

  for (int order = 0; order <= MAX_ORDER; order++)
  {
    list_init(&buddy_zone.free_lists[order].free_list_head);
    buddy_zone.free_lists[order].free_block_count = 0;
  }

  uint64_t base_frame = buddy_zone.heap_base / PAGE_SIZE;
  for (uint64_t i = 0; i < num_pages; i++)
  {
    buddy_zone.pages[i].frame_number = base_frame + i;
    buddy_zone.pages[i].flags = 0;
    buddy_zone.pages[i].order = 0;
    list_init(&buddy_zone.pages[i].free_list_node);
  }
  buddy_add_memory(num_pages);
}

void mm_get_stats(uint64_t *total, uint64_t *free)
{
  if (total)
    *total = buddy_zone.total_pages * PAGE_SIZE;
  if (free)
  {
    uint64_t free_pages = 0;
    for (int o = 0; o <= MAX_ORDER; o++)
    {
      free_pages += (buddy_zone.free_lists[o].free_block_count << o);
    }
    *free = free_pages * PAGE_SIZE;
  }
}

const char *mm_get_name(void)
{
  return "Buddy System";
}

#endif
