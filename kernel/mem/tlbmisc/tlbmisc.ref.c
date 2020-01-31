/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
/* #include <tlbfmt.h> */
/* #include <max.h> */
#include <hw.h>
/* #include <symbols.h> */
/* #include <asid.h> */
/* #include <linear.h> */
/* #include <pagewalk.h> */
#include <globals.h>

void H2K_mem_tlb_invalidate_asid(u32_t asid) {

	/* Don't need the tmpmap_lock here because the tmp mapping has global bit set
		 and tlbinvasid supposedly leaves global entries alone */
	asid <<=  (32 - PAGE_BITS);
	H2K_mutex_lock_tlb();
	asm volatile
		(
		 " tlbinvasid(%0)\n"
		 :
		 : "r"(asid)
		 );
	H2K_isync();
	H2K_mutex_unlock_tlb();

#if ARCHV >= 68
	if (H2K_gp->dma_version > 0) {  // have dma
		H2K_dmtlbsynch();
	}
#endif
}

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me)
{
	u32_t tmp;
	if (count > 1) return H2K_mem_tlb_invalidate_asid(asid);

	H2K_spinlock_lock(&H2K_gp->tmpmap_lock);  // for tmpmap
	H2K_mutex_lock_tlb();  // for H2K_safemem_check_and_lock
	tmp = H2K_mem_tlb_probe(va,asid);
	if (tmp <= H2K_gp->last_tlb_index) {
		H2K_mem_tlb_write(tmp,0);
		H2K_isync();
	}
	H2K_mutex_unlock_tlb();
	H2K_spinlock_unlock(&H2K_gp->tmpmap_lock);

#if ARCHV >= 68
	if (H2K_gp->dma_version > 0) {  // have dma
		H2K_dmtlbsynch();
	}
#endif
}
