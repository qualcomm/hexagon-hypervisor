/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_map.h>
#include <h2_vmtraps.h>
#include <q6protos.h>

//  Process for setting this translation up is still a little
//  up in the air.  Need to make sure that the memory for this 
//  translation table is pinned.

lin_map_fmt_t translations[MAX_TRANSLATIONS+1];

//  Might belong in a header file elsewhere
#define get_page_size(ord) (4096 * (1 << ((ord)*2)))

static inline lin_map_fmt_t map2linear(unsigned long vpn, int size , int prot, 
	int flags, unsigned long ppn)
{
	lin_map_fmt_t ret;
	
	//  low word
	ret.ppn = ppn;
	ret.cccc = get_cccc(flags);
	ret.xwru = (prot & PROT_READ) || (prot & PROT_WRITE) || (prot & PROT_EXEC);

	//  high word
	ret.vpn = vpn;
	ret.size = size;
	ret.chain = 0;

	return ret;
}

int intersects(lin_map_fmt_t *entry, unsigned long start_vpn, unsigned long page_size)
{
	unsigned long head;
	unsigned long tail;  	//  actually it's end page + 1

	//  Interesting cases:  
	//  start addresses are the same
	//  normal intersection
	//  completely contained

	if (entry->vpn > start_vpn) {
		head = entry->vpn;
		tail = start_vpn+(page_size>>12);
	} 
	else {
		head = start_vpn;
		tail = entry->vpn+(get_page_size(entry->size)>>12);
	} 
	if (head <= tail-1) {
		return 1;
	}
	return 0;
}

void *h2_map(void *va,size_t length,int prot,int flags,void *pa)
{
	int i;
	int free=-1;
	int size_ord;		//  ordinal of the size
	int page_size;
	void *aligned_va;
	unsigned long aligned_vpn;

	/* 
	 * VA will be aligned to the smallest page that fits the length
	 * 	if you don't specify MAP_FIXED.  
	 * Non-aligned VA with MAP_FIXED = FAIL.
	 * Aligned VA + length which overlaps another mapping = FAIL.
	 */

	if ((length > 1<<24) || (length == 0)) return NULL;

	//  this seems like a not very elegant way to do this.
	size_ord = 32-Q6_R_cl0_R(length);
	size_ord = size_ord <= 12 ? 0 : (size_ord-11)>>2;  // technically -12+1

	page_size = get_page_size(size_ord);

	aligned_va = (void *)(~(page_size-1) * (unsigned long) va);
	aligned_vpn = (unsigned long) aligned_va >> 12;

	if ((flags & MAP_FIXED) && (va != aligned_va))
		return NULL;

	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		if (translations[i].raw == 0) {
			return NULL;
		}  //  zero entry means stop
		if (translations[i].high == 0) {
			free=i;
		}
		//  TODO:  don't check if .high == 0?
		if (intersects(&translations[i],aligned_vpn,page_size)) {
			return NULL;
		}
	}

	if (free == -1) {
		return NULL;
	}
	else {
		translations[free] = map2linear(aligned_vpn,size_ord,prot,flags,(unsigned long) pa >> 12);
	}
	return aligned_va;
}

void h2_unmap(void *va)
{
	int i;

	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		if (translations[i].raw == 0) {
			return;
		}
		//  TODO:  don't check if .high == 0?
		if (intersects(&translations[i],(unsigned long) va, 4096)) {
			translations[i].low = (unsigned long) &translations[i+1]; 
			translations[i].high = 0;
			h2_vmtrap_clrmap(va);
		}
	}
}
