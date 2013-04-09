/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_PHYSMEM_H
#define H2OS_PHYSMEM_H 1

#include <vm.h>

/*
 * We can keep track of physical memory, mainly indexed by either the VA where it is mapped,
 * or the PA where it really lives 
 */

/*
 * If we choose VA:
 * 
 * Although this is technically keeping track of *physical* memory,
 * we require that all physical memory that we can use be mapped into 
 * kernel memory space permanently
 * 
 * Because of this restriction, we keep track of pages here based only
 * on their virtual (mapped) address
 * 
 * For memory with discontiguous physical addresses, the only requirement is
 * that the memory be mapped into the kernel memory space.
 * 
 * However, that makes VA2PA and PA2KVA mapping more complex
 * 
 * Of course you have to pay the cost somewhere.  So when we're reading or
 * modifying page tables we need to be able to convert pa2va and va2pa.
 * 
 * If we require that memory be mapped in some granularity -- say, contiguous
 * 4MB blocks -- we should be able to make simple lookup tables for va2pa and
 * pa2va.  Small discontiguous memories won't fill in everything, but that's
 * OK.
 * 
 * EJP: Today I think this is preferable
 * 
 */

/*
 * If we choose PA instead:
 * 
 * Since the buddy allocator keeps track of unused pages using the memory 
 * in the pages themselves (though we could change that I guess), we need 
 * to ensure that free pages are mapped into the VA space somewhere
 * 
 * Here, we keep track of pages here based only on their physical address
 * 
 * For memory with discontiguous physical addresses, we will use a 2-level
 * lookup table.  Invalid regions are marked as such at the upper level,
 * and at the lower level, invalid pages are marked as such (non-free).
 * 
 * Of course you have to pay the cost somewhere.  So when we're
 * manipulating a free page or the contents of a kernel-owned page, it
 * needs to be mapped somewhere.  For most memory we can keep things mapped
 * all the time, but discontiguous memories may need to live somewhere special.
 * We might think that we need to do the highmem thing like Linux, but probably
 * we will go to bigger VA spaces before we need to worry about huge PA spaces
 * in this OS, and so we can probably assume that all memory is mapped.
 * 
 * If we require that memory be mapped in some granularity -- say, contiguous
 * 4MB blocks -- we should be able to make simple lookup tables for va2pa and
 * pa2va.  Small discontiguous memories won't fill in everything, but that's
 * OK.
 */

#include <c_std.h>
#include <vm.h>

/* 
 * Type for this phyiscal page 
 * This enumeration may need to be changed,
 * For example we may want to record mmap'd files
 * And those mmap'd files may be private or shared
 * In any case, 15 types is hopefully enough...
 */
enum {
	PHYS_PAGE_FREE,
	PHYS_PAGE_RWSHARED,
	PHYS_PAGE_COW,
	PHYS_PAGE_PRIVATE,
	PHYS_PAGE_INVALID,
};

/*
 * Type for a ppage
 * We want a generous number of bits for reference counting
 * Additionally, we want to store order of the page (buddy allocation) and what the
 * status of this page is
 */

typedef union {
	u32_t raw;
	struct {
		u32_t count:24;		/* Reference Count */
		u32_t order:4;		/* Order for this page */
		u32_t status:4;		/* How page is used */
	};
} ppage_t;

extern ppage_t pages[];

#if 0
/*
 * Converting VA to PA and back for kernel memory
 * 
 * If kernel offset is too simple:
 * 
 * We use the upper bits of the address to index into a table
 * The value at the table is xor'd with the upper bits to form an address
 * This gives us more flexibility in how things are mapped than a single kern phys offset
 */ 

extern s16_t va_maptab[];
extern s16_t pa_maptab[];

static inline pa_t va_to_pa(void *va)
{
	pa_t va_i = (pa_t)(unsigned long)va;
	va_i ^= va_maptab[va_i >> BIGPAGE_OFFSET & ((1<<(32-BIGPAGE_OFFSET))-1)] << BIGPAGE_OFFSET;
	return va_i;
}

static inline void *pa_to_va(pa_t pa)
{
	pa ^= pa_maptab[pa >> BIGPAGE_OFFSET & ((1<<(32-BIGPAGE_OFFSET))-1)] << BIGPAGE_OFFSET;
	return (void *)(long)pa;
}
#else
/* 
 * Converting VA to PA and back for kernel memory
 * Use a simple offset
 */
static inline pa_t va_to_pa(void *va)
{
	pa_t pa = (pa_t)(unsigned long)va;
	pa -= KERN_OFFSET;
	return pa;
}
static inline void *pa_to_va(pa_t pa)
{
	pa += KERN_OFFSET;
	return (void *)(long)pa;
}
#endif

static inline ppage_t *va_to_ppage(void *va)
{
	u32_t va_raw = (u32_t)va;
	u32_t idx = (va_raw - KERN_OFFSET) >> PAGE_SHIFT;
	return &pages[idx];
}

static inline void *ppage_to_va(ppage_t *ppage)
{
	u32_t idx = (ppage - pages);
	u32_t va_raw = KERN_OFFSET + (idx << PAGE_SHIFT);
	return (void *)va_raw;
}

void physmem_init();
void physmem_add_4k_page(void *addr);

#endif
