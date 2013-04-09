/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_ARMPT_H
#define H2OS_ARMPT_H 1

/*
 * We make some choices here:
 * * We are choosing ARM V7L format page tables
 * * We are choosing 2GB TTBR0 for user, 2GB TTBR1 for guest
 * * We have MAIR set up as .... 0 == WB cacheable, 7 = Device
 */ 

#ifndef MM_T_DEFINED
#error you must define mm_t before including this file
#endif

#include <c_std.h>

typedef struct pagetable_mm_info_struct {
	pte_t s1ptetable[2];
	u64_t ttbr0;
} pagetable_mm_info_t;

static inline paddr_t get_base(mm_t *mm, vaddr_t va)
{
	mm->pagetable_info.ttbr0 & 0x0000FFFFFFFFFFF0ULL
}

typedef union {
	u64_t raw;
	struct {
		u64_t valid:1;
		u64_t table:1;
		u64_t rest:62;
	};
	struct {
		u64_t one:2;
		u64_t lower:10;
		u64_t unused:18;
		u64_t base:22;
		u64_t upper:12;
	} block_1g;
	struct {
		u64_t one:2;
		u64_t lower:10;
		u64_t unused:9;
		u64_t base:31;
		u64_t upper:12;
	} block_2m;
	struct {
		u64_t three:2;
		u64_t lower:10;
		u64_t base:40;
		u64_t upper:12;
	} page;
	struct {
		u64_t three:2;
		u64_t ignored0:10;
		u64_t baseaddr:40;
		u64_t ignored1:7;
		u64_t tableperms:5;
	} tab;
	struct {
		u64_t type:2;
		u64_t AttrIndex:3;
		u64_t ns:1;
		u64_t ap21:2;
		u64_t sh10:2;
		u64_t af:1;
		u64_t ng:1;
		u64_t address:40;
		u64_t contig:1;
		u64_t pxn:1;
		u64_t xn:1;
		u64_t rsvd:4;
		u64_t ignored:5;
	} attribs;
	struct {
		u64_t three:2;
		u64_t ignored0:10;
		u64_t baseaddr:40;
		u64_t ignored1:7;
		u64_t rsvd:5;
	} s2table;
	struct {
		u64_t type:2;
		u64_t MemAttr:4;
		u64_t hap10:2;
		u64_t sh10:2;
		u64_t af:1;
		u64_t rsvd0_0:1;
		u64_t address:40;
		u64_t contig:1;
		u64_t rsvd0_1:1;
		u64_t xn:1;
		u64_t rsvd:4;
		u64_t ignored:5;
	} s2attribs;
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
	if ((offset > 12) && (pte.tab.three == 3)) {
		paddr_t newbase = entry.tab.baseaddr<<12;
		return pagewalk_step(mm,va,handler,opaque,9,offset-9,newbase);
	} else {
		/* invalid or page or block */
		handler(va,opaque,ptep,offset);
	}
}

static inline long pagewalk(mm_t *mm, vaddr_t va, const handler_t handler, void *opaque)
{
	paddr_t base = get_base(mm,va);
	width = 1;	// XXX: FIXME: be smart / configurable? at least #define?
	offset = 30;	// XXX: FIXME: be smart / configurable? at least #define?
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
		if ((offset > 12) && (pte.tab.three == 3)) {
			paddr_t newbase = entry.tab.baseaddr << 12;
			pagevisit_level(mm,cur,end,handler,opaque,9,offset-9,newbase);
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
	paddr_t base = get_base(mm,start);
	width = 1;	// XXX: FIXME: be smart / configurable? at least #define?
	offset = 30;	// XXX: FIXME: be smart / configurable? at least #define?
	pagevisit_level(mm,start,handler,opaque,width,offset,base);
}

static inline int pte_valid(pte_t pte) { return pte.valid; }
static inline paddr_t pte_pa(pte_t pte) { return pte.page.base << 12; }
static inline int pte_r(pte_t pte) { return ((pte.attribs.ap21 & 1) != 0); }
static inline int pte_u(pte_t pte) { return ((pte.attribs.ap21 & 1) != 0); }
static inline int pte_w(pte_t pte) { return ((pte.attribs.ap21 & 2) == 0); }
static inline int pte_x(pte_t pte) { return ((pte.nx) == 0); }
static inline pte_t pte_mkr(pte_t pte) { pte.attribs.ap21 |= 1; return pte; }
static inline pte_t pte_mku(pte_t pte) { pte.attribs.ap21 |= 1; return pte; }
static inline pte_t pte_mkw(pte_t pte) { pte.attribs.ap21 &= 1; return pte; }
static inline pte_t pte_mkx(pte_t pte) { pte.attribs.nx = 0; return pte; }
static inline pte_t pte_mkwp(pte_t pte) { pte.attribs.ap21 |= 2; return pte; }
static inline pte_t pte_set_pa(pte_t pte, paddr_t pa) { pte.page.base = pa >> 12; return pte; }

#define PTE_TYPE_OFF 0
#define PTE_TYPE_VALID_4K (0x3ULL << PTE_TYPE_OFF)
#define PTE_TYPE_VALID_2M (0x1ULL << PTE_TYPE_OFF)
#define PTE_TYPE_TABLE    (0x3ULL << PTE_TYPE_OFF)
#define PTE_ATTRIDX_OFF 2
#define PTE_ATTR_DEFAULT  (0x0ULL << PTE_ATTRIDX_OFF)
/* Add: Device type */
#define PTE_SECURITY_OFF 5
#define PTE_SECURITY_S    (0x0ULL << PTE_SECURITY_OFF)
#define PTE_SECURITY_NS   (0x1ULL << PTE_SECURITY_OFF)
#define PTE_RWPERM_OFF 6
#define PTE_RWPERM_WRU    (0x1ULL << PTE_RWPERM_OFF)
#define PTE_RWPERM_WR     (0x0ULL << PTE_RWPERM_OFF)
#define PTE_RWPERM_RU     (0x3ULL << PTE_RWPERM_OFF)
#define PTE_RWPERM_R      (0x2ULL << PTE_RWPERM_OFF)
#define PTE_SHARE_OFF 8
#define PTE_SHARE_NONE    (0x0ULL << PTE_SHAR_OFF)
#define PTE_SHARE_OUTER   (0x2ULL << PTE_SHAR_OFF)
#define PTE_SHARE_INNER   (0x3ULL << PTE_SHAR_OFF)
#define PTE_AF_OFF 10
#define PTE_AF_NOA        (0x0ULL << PTE_AF_OFF)
#define PTE_AF_AOK        (0x1ULL << PTE_AF_OFF)
#define PTE_NG_OFF 11
#define PTE_NG_GLOBAL     (0x0ULL << PTE_NG_GLOBAL)
#define PTE_NG_NONGLOBAL  (0x1ULL << PTE_NG_GLOBAL)
#define PTE_ADDRESS_OFF 12
#define PTE_CONTIG_OFF 52
#define PTE_CONTIG_16     (0x1ULL << PTE_CONTIG_OFF)
#define PTE_PXNPERM_OFF 53
#define PTE_PXNPERM_NX    (0x1ULL << PTE_PXNPERM_OFF)
#define PTE_PXNPERM_X     (0x0ULL << PTE_PXNPERM_OFF)
#define PTE_XNPERM_OFF 54
#define PTE_XNPERM_NX     (0x1ULL << PTE_XNPERM_OFF)
#define PTE_XNPERM_X      (0x0ULL << PTE_XNPERM_OFF)
// 55,56,57,58 rsvd, 59,60,61,62,63 ignored

#define PTE_PERM_R (PTE_RWPERM_R | PTE_XNPERM_NX | PTE_PXNPERM_NX)
#define PTE_PERM_RW (PTE_RWPERM_RW | PTE_XNPERM_NX | PTE_PXNPERM_NX)
#define PTE_PERM_RX (PTE_RWPERM_R)
#define PTE_PERM_RWX (PTE_RWPERM_RW)

#define PTE_PERM_RU (PTE_RWPERM_RU | PTE_XNPERM_NX | PTE_PXNPERM_NX)
#define PTE_PERM_RWU (PTE_RWPERM_RWU | PTE_XNPERM_NX | PTE_PXNPERM_NX)
#define PTE_PERM_RXU (PTE_RWPERM_RU)
#define PTE_PERM_RWXU (PTE_RWPERM_RWU)

#define PTE_DEFAULT_4K (PTE_TYPE_VALID_4K | PTE_PERM_RWXU | PTE_ATTR_DEFAULT | \
		PTE_SECURITY_NS | PTE_SHARE_INNER | PTE_AF_AOK | PTE_NG_NONGLOBAL)
#define PTE_DEFAULT_2M (PTE_TYPE_VALID_2M | PTE_PERM_RWXU | PTE_ATTR_DEFAULT | \
		PTE_SECURITY_NS | PTE_SHARE_INNER | PTE_AF_AOK | PTE_NG_NONGLOBAL)
#define PTE_DEFAULT_TAB (PTE_TYPE_TABLE)

#define PTE_MAKE_DEFAULT_4K(X) (((X) & 0x000FFFFFFFFFF000ULL) | PTE_DEFAULT_4K)
#define PTE_MAKE_DEFAULT_2M(X) (((X) & 0x000FFFFFFFFFF000ULL) | PTE_DEFAULT_2M)
#define PTE_MAKE_DEFAULT_TAB(X) (((X) & 0x000FFFFFFFFFF000ULL) | PTE_DEFAULT_TAB)

static inline pte_t pte_default_4k(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_4K(x); return r;}
static inline pte_t pte_default_2m(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_2M(x); return r;}
static inline pte_t pte_default_tab(paddr_t x) {pte_t r; r.raw=PTE_MAKE_DEFAULT_TAB(x); return r;}

#define SMALLPAGESIZE 12
#define BIGPAGESIZE 21

#define pte_default_small pte_default_4k
#define pte_default_big pte_default_2m

#endif
