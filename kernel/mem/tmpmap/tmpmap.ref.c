/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tlbfmt.h>
#include <globals.h>
#include <spinlock.h>
#include <hw.h>
#include <tlbfill.h>
#include <tlbmisc.h>

static u32_t tmpmap_lock IN_SECTION(".data.core.globals");

/* Return va of pa */
u32_t H2K_tmpmap_add_and_lock(u32_t pa, u32_t cccc) {

	H2K_mem_tlbfmt_t entry;
	u32_t index;

	entry.raw = 0;
	entry.ppd = ((pa & TEMP_MAP_PG_MASK ) >> (PAGE_BITS - 1)) | (1 << TEMP_MAP_PG_SIZE);
	entry.cccc = cccc;
	entry.xwru = 0;  // only monitor access
	entry.vpn = (TEMP_MAP_VA >> PAGE_BITS);
	// entry.asid = don't care
	entry.global = 1;
	entry.valid = 1;

	/* Hold this lock until the caller calls H2K_tmpmap_remove_and_unlock() */
	H2K_spinlock_lock(&tmpmap_lock);

	/* Lock TLB and allocate the last entry */
	H2K_mutex_lock_tlb();
	index = H2K_gp->last_tlb_index--;
	H2K_mem_tlb_insert_index_unlock(entry, index);  // and invalidate what's there
	return TEMP_MAP_VA | (pa & TEMP_MAP_OFF_MASK);
}

void H2K_tmpmap_remove_and_unlock() {

	H2K_mutex_lock_tlb();
	H2K_mem_tlb_write(++(H2K_gp->last_tlb_index), 0);  // invalidate tmpmap
	H2K_mutex_unlock_tlb();
	H2K_spinlock_unlock(&tmpmap_lock);
}

void H2K_tmpmap_init() {

	H2K_spinlock_init(&tmpmap_lock);
}
