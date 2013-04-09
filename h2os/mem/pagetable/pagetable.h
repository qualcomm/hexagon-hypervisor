/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _H2OS_PAGETABLE_H
#define _H2OS_PAGETABLE_H 1

#ifdef PAGETABLE_HVM_FORMAT
#include "pagetable_hvm.h"
#else
#include "pagetable_arm.h"
#endif

/*
 * Add a PTE at the specified location and size
 * 
 * If this can't be done, returns nonzero
 */
int pagetable_insert_pte(mm_t *mm, vaddr_t va, int size, pte_t pte);

/*
 * Try to allocate a small page, add at specified location 
 */
int pagetable_create_small(mm_t *mm, vaddr_t va);

/*
 * Try to allocate a big page, add at specified location 
 */
int pagetable_create_big(mm_t *mm, vaddr_t va);

#ifdef DEBUG
#include <stdio.h>

static inline long pagetable_dump_printer(vaddr_t va, void *opaque, pte_t *ptep, int offset)
{
	printf("va=0x%08x: size=%d: raw=0x%016llx ",va,offset,(u64_t)(*ptep));
	if (pte_valid(*ptep)) {
		printf("valid,pa=0x%016llx ",pte_pa(*ptep));
	} else {
		printf("invalid\n");
		return 1;
	}
	if (pte_r(*ptep)) printf("r"); else printf(" ");
	if (pte_w(*ptep)) printf("w"); else printf(" ");
	if (pte_x(*ptep)) printf("x"); else printf(" ");
	return 1;
}
static inline void pagetable_dump(mm_t *mm) {
	pagevisit(mm,0,0xFFFFFFFF,pagetable_dump_printer,NULL);
}

#else
static inline void pagetable_dump(mm_t *mm) {} 
#endif

#endif
