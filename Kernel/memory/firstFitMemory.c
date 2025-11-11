

#ifdef FIRSTFIT

#include <stdint.h>
#include "../include/lib.h"
#include <stddef.h>

#define KHEAPFLAG_USED 0x80000000

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
} KHEAPBLOCKLCAB;

typedef struct _KHEAPLCAB
{
    KHEAPBLOCKLCAB *fblock;
    uint32_t bcnt;
} KHEAPLCAB;

static KHEAPLCAB kernel_heap;

void k_heapLCABInit(KHEAPLCAB *heap)
{
    heap->fblock = 0;
    heap->bcnt = 0;
}

int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr_t addr, uint32_t size)
{
    KHEAPBLOCKLCAB *hb = (KHEAPBLOCKLCAB *)addr;
    KHEAPHDRLCAB *hdr;

    hb->size = size;
    hb->used = sizeof(KHEAPBLOCKLCAB) + sizeof(KHEAPHDRLCAB);

    hb->next = heap->fblock;
    heap->fblock = hb;

    hdr = (KHEAPHDRLCAB *)&hb[1];

    hdr->flagsize = hb->size - (sizeof(KHEAPBLOCKLCAB) + 32);

    hdr->prevsize = 0;

    ++heap->bcnt;

    return 1;
}

void *k_heapLCABAlloc(KHEAPLCAB *heap, uint32_t size)
{
    KHEAPBLOCKLCAB *hb;
    KHEAPHDRLCAB *hdr, *_hdr;
    uint32_t sz;
    uint8_t fg;
    uint32_t checks = 0;
    uint32_t bc = 0;

    for (hb = heap->fblock; hb; hb = hb->next)
    {

        if ((hb->size - hb->used) >= (size + sizeof(KHEAPHDRLCAB)))
        {
            ++bc;

            hdr = (KHEAPHDRLCAB *)&hb[1];

            while ((uintptr_t)hdr < ((uintptr_t)hb + hb->size))
            {
                ++checks;

                fg = hdr->flagsize >> 31;

                sz = hdr->flagsize & 0x7fffffff;

                if (!fg)
                {
                    if (sz >= size)
                    {

                        if (sz > (size + sizeof(KHEAPHDRLCAB) + 16))
                        {

                            _hdr = (KHEAPHDRLCAB *)((uintptr_t)&hdr[1] + size);

                            _hdr->flagsize = sz - (size + sizeof(KHEAPHDRLCAB));
                            _hdr->prevsize = size;

                            hdr->flagsize = KHEAPFLAG_USED | size;

                            hb->used += sizeof(KHEAPHDRLCAB);
                        }
                        else
                        {

                            hdr->flagsize |= KHEAPFLAG_USED;
                        }

                        hb->used += size;

                        return &hdr[1];
                    }
                }

                hdr = (KHEAPHDRLCAB *)((uintptr_t)&hdr[1] + sz);
            }
        }
    }

    return 0;
}

void k_heapLCABFree(KHEAPLCAB *heap, void *ptr)
{
    KHEAPHDRLCAB *hdr, *phdr, *nhdr;
    KHEAPBLOCKLCAB *hb;
    uint32_t sz;

    for (hb = heap->fblock; hb; hb = hb->next)
    {
        if (((uintptr_t)ptr > (uintptr_t)hb) &&
            ((uintptr_t)ptr < (uintptr_t)hb + hb->size))
        {

            hdr = (KHEAPHDRLCAB *)((uintptr_t)ptr - sizeof(KHEAPHDRLCAB));

            hdr->flagsize &= ~KHEAPFLAG_USED;

            uint32_t size_to_free = (hdr->flagsize & 0x7fffffff);
            if (hb->used >= size_to_free)
            {
                hb->used -= size_to_free;
            }
            else
            {
                hb->used = 0;
            }

            if (hdr->prevsize)
            {

                phdr = (KHEAPHDRLCAB *)((uintptr_t)hdr -
                                        (sizeof(KHEAPHDRLCAB) + hdr->prevsize));
            }
            else
            {
                phdr = 0;
            }

            sz = hdr->flagsize & 0x7fffffff;
            nhdr = (KHEAPHDRLCAB *)((uintptr_t)&hdr[1] + sz);

            if ((uintptr_t)nhdr >= ((uintptr_t)hb + hb->size))
            {
                nhdr = 0;
            }

            if (nhdr)
            {
                if (!(nhdr->flagsize & KHEAPFLAG_USED))
                {

                    hdr->flagsize += sizeof(KHEAPHDRLCAB) +
                                     (nhdr->flagsize & 0x7fffffff);

                    if (hb->used >= sizeof(KHEAPHDRLCAB))
                    {
                        hb->used -= sizeof(KHEAPHDRLCAB);
                    }
                    else
                    {
                        hb->used = 0;
                    }

                    nhdr = (KHEAPHDRLCAB *)((uintptr_t)&hdr[1] +
                                            (hdr->flagsize & 0x7fffffff));
                    if ((uintptr_t)nhdr < ((uintptr_t)hb + hb->size))
                    {
                        nhdr->prevsize = hdr->flagsize & 0x7fffffff;
                    }
                }
            }

            if (phdr)
            {
                if (!(phdr->flagsize & KHEAPFLAG_USED))
                {

                    phdr->flagsize += sizeof(KHEAPHDRLCAB) +
                                      (hdr->flagsize & 0x7fffffff);

                    if (hb->used >= sizeof(KHEAPHDRLCAB))
                    {
                        hb->used -= sizeof(KHEAPHDRLCAB);
                    }
                    else
                    {
                        hb->used = 0;
                    }

                    hdr = phdr;

                    nhdr = (KHEAPHDRLCAB *)((uintptr_t)&hdr[1] +
                                            (hdr->flagsize & 0x7fffffff));
                    if ((uintptr_t)nhdr < ((uintptr_t)hb + hb->size))
                    {
                        nhdr->prevsize = hdr->flagsize & 0x7fffffff;
                    }
                }
            }

            return;
        }
    }
}

void *mm_alloc(uint32_t size)
{
    return k_heapLCABAlloc(&kernel_heap, size);
}

void mm_free(void *ptr)
{
    k_heapLCABFree(&kernel_heap, ptr);
}

void mm_init(uintptr_t start, uint32_t size)
{
    k_heapLCABInit(&kernel_heap);
    k_heapLCABAddBlock(&kernel_heap, start, size);
}

void mm_get_stats(uint64_t *total, uint64_t *free)
{
    *total = 0;
    *free = 0;

    KHEAPBLOCKLCAB *block = kernel_heap.fblock;
    while (block != NULL)
    {
        *total += block->size;
        *free += (block->size - block->used);
        block = block->next;
    }
}

const char *mm_get_name(void)
{
    return "First-Fit";
}

#endif
