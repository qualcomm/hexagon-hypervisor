/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tlbfmt.h>
#include <hw.h>

/* Called by tmpmap */
void H2K_mem_tlb_insert_index_unlock(H2K_mem_tlbfmt_t entry, u32_t index) {

	u64_t rawentry = entry.raw;
	u32_t result;

#if ARCHV < 60
	u32_t tag = entry.vpn | (entry.asid << (32 - PAGE_BITS));
#endif

	asm volatile
		(
#if ARCHV >= 60
		 " %0 = ctlbw(%1,%2)\n"
#else // < V60
		 " %0 = tlbp(%3)\n"
		 " p0 = tstbit(%0, #31)\n"
		 " if (!p0) jump 1f\n"
		 " tlbw(%1,%2)\n"
#endif
		 " isync\n"
		 "1:\n"
		 : "=&r" (result)
		 : "r"(rawentry),
			 "r"(index)
#if ARCHV < 60
			 ,"r"(tag)
		 : "p0"
#endif
		 );

	H2K_mutex_unlock_tlb();
}

