/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_ARMPT_H
#define H2OS_ARMPT_H 1

#include <c_std.h>

typedef union {
	u32_t raw;
	struct {
		u32_t ignored:3;
		u32_t ignored2:1;
		u32_t trusted:1;
		u32_t user:1;
		u32_t ccc:3;
		u32_t r:1;
		u32_t w:1;
		u32_t x:1;
		u32_t ppn:20;
	} page;
	struct {
		u32_t five:3;
		u32_t unused:1;
		u32_t user:1;
		u32_t ccc:3;
		u32_t r:1;
		u32_t w:1;
		u32_t x:1;
		u32_t lowbits:10;
		u32_t ppn:10;
	} block_4m;
	struct {
		u32_t zero:3;
		u32_t unused:1;
		u32_t ignored:8;
		u32_t tablebase:20;
	} tab;
	struct {
		u32_t seven:3;
		u32_t ignored:29;
	} invalid;
} pte_t;

static inline long pagewalk_step(mm_t *mm,
	vaddr_t va,
	const handler_t handler,
	void *opaque,
	int width,
	int offset, // XXX: FIXME: instead of offset, do we want order?
	paddr_t base)
{
	paddr_t pa;
	pte_t *ptep;
	pte_t pte;
	pa = tableidx(base,va,width,offset,3);
	ptep = pa2va(pa);
	pte = *ptep;
	if ((offset > 12) && (pte.tab.zero == 0)) {
		paddr_t newbase = entry.tab.tablebase<<12;
		return pagewalk_step(mm,va,handler,opaque,10,offset-10,newbase);
	} else {
		/* invalid or page or block */
		handler(va,opaque,ptep,offset);
	}
}

static inline long pagewalk(mm_t *mm, vaddr_t va, const handler_t handler, void *opaque)
{
	paddr_t base = get_base(mm->ttbr0,va);
	width = 10;	// XXX: FIXME: be smart / configurable? at least #define?
	offset = 22;	// XXX: FIXME: be smart / configurable? at least #define?
	return pagewalk_step(mm,va,handler,opaque,width,offset,base);
}

static inline void pagevisit_level(mm_t *mm, 
	vaddr_t cur,
	vaddr_t end, 
	const handler_t handler, 
	void *opaque,
	int width,
	int offset,
	paddr_t base)
{
	paddr_t pa;
	pte_t *ptep;
	pte_t pte;
	pa = tableidx(base,cur,width,offset,3);
	ptep = pa2va(pa);
	int i;
	for (i = 0; i < (1<<width); i++) {
		if (cur >= end) return;
		pte = *ptep;
		if ((offset > 12) && (pte.tab.zero == 0)) {
			paddr_t newbase = entry.tab.tablebase<<12;
			return pagevisit_level(mm,cur,end,handler,opaque,10,offset-10,newbase);
		} else {
			handler(cur,opaque,ptep,offset);
		}
		cur += (1L<<offset)
		ptep++;
	}
}

static inline void pagevisit(mm_t *mm, 
	vaddr_t start,
	vaddr_t end, 
	const handler_t handler, 
	void *opaque)
{
	paddr_t base = get_base(mm->tablebase,start);
	width = 10;	// XXX: FIXME: be smart / configurable? at least #define?
	offset = 22;	// XXX: FIXME: be smart / configurable? at least #define?
	pagevisit_level(mm,start,handler,opaque,width,offset,base);
}

static inline int pte_valid(pte_t pte) { return (pte.invalid.seven != 7) && (pte.page.r || pte.page.w || pte.page.x); }
static inline int pte_pa(pte_t pte) { return pte.page.ppn << 12; }
static inline int pte_r(pte_t pte) { return pte.page.r; }
static inline int pte_w(pte_t pte) { return pte.page.w; }
static inline int pte_x(pte_t pte) { return pte.page.x; }
static inline pte_t pte_mkr(pte_t pte) { pte.page.r = 1; return pte; }
static inline pte_t pte_mkw(pte_t pte) { pte.page.w = 1; return pte; }
static inline pte_t pte_mkx(pte_t pte) { pte.page.x = 1; return pte; }
static inline pte_t pte_mkwp(pte_t pte) { pte.page.w = 0; return pte; }

#define PTE_DEFAULT_4K 0x00000ff0
#define PTE_DEFAULT_4M 0x00000ff5
#define PTE_DEFAULT_TAB 0x00000ff0	// compatible with PTE_DEFAULT_4K for self-mapping, if we even want that

#define PTE_MAKE_DEFAULT_4K(X) (((X) & 0xFFFFF000UL) | PTE_DEFAULT_4K)
#define PTE_MAKE_DEFAULT_4M(X) (((X) & 0xFFFFF000UL) | PTE_DEFAULT_4M)
#define PTE_MAKE_DEFAULT_TAB(X) (((X) & 0xFFFFF000UL) | PTE_DEFAULT_TAB)

static inline pte_t pte_default_4k(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_4K(x); return r;}
static inline pte_t pte_default_4m(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_4M(x); return r;}
static inline pte_t pte_default_tab(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_TAB(x); return r;}

#define SMALLPAGESIZE 12
#define BIGPAGESIZE 22

#define  pte_default_small pte_default_4k
#define  pte_default_big pte_default_4m

#endif
