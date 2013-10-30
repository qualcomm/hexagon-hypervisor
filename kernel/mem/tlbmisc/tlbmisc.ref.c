/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <max.h>
#include <hw.h>
#include <symbols.h>
#include <asid.h>
#include <linear.h>
#include <pagewalk.h>

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me)
{
	u32_t tmp;
	u32_t start = va >> PAGE_BITS;
	u32_t end = (va + count - 1) >> PAGE_BITS;
	u32_t page;
	H2K_mem_tlbfmt_t entry;
	u32_t i;
	u32_t size, gsize = 0;
	H2K_mem_tlbfmt_t (*get_fn)(u32_t badva, H2K_thread_context *me);

	/* We don't lock the TLB here to make the read/probe atomic with the write.
		 It's possible that a TLB miss will replace the entry that we just decided
		 to invalidate but that should be safe --- at most just cause another TLB
		 miss. */

	/* Look up the translation for the first va to get the page size */
	/* FIXME: Also look up the guest->phys translation in case that page size is
		 smaller? */
	switch (H2K_mem_asid_table[asid].fields.transtype) {
	case H2K_ASID_TRANS_TYPE_LINEAR:
		get_fn = H2K_mem_get_linear;
		break;

	case H2K_ASID_TRANS_TYPE_TABLE:
		get_fn = H2K_mem_get_pagetable;
		break;

	case H2K_ASID_TRANS_TYPE_OFFSET:
		get_fn = H2K_vm_get_offset;
		break;

	default:
		return;
	}

	/* Heuristic: If the range is "big", dump TLB and check each entry, else
		 probe for each 4K page in the range */
	if ((entry = get_fn(va,me)).raw != 0) {
		gsize = H2K_mem_tlbfmt_get_size(entry) * 2;
	}

	if ((end >> gsize) - (start >> gsize) > MAX_TLB_ENTRIES) {
		for (i = ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1; i < MAX_TLB_ENTRIES; i++) {
			entry.raw = H2K_mem_tlb_read(i);
			size = 0x1 << (H2K_mem_tlbfmt_get_size(entry) * 2 + PAGE_BITS);
			if (entry.vpn >= start && entry.vpn + size - 1 <= end) { // in range
				H2K_mem_tlb_write(i, 0);
			}
		}
	} else {
		for (page = start; page <= end; page++) {
			H2K_TLB_ATOMIC_START;
			tmp = H2K_mem_tlb_probe(page << PAGE_BITS, asid);
			H2K_TLB_ATOMIC_END;
			if (((tmp >> 31) & 1) == 0) {
				H2K_TLB_ATOMIC_START;
				H2K_mem_tlb_write(tmp,0);
				H2K_TLB_ATOMIC_END;
			}
#if ARCHV <= 3
			/* For V3 and earlier, also need to probe fake guest bit */
			tmp = H2K_mem_tlb_probe(page << PAGE_BITS, asid|0x20);
			if (((tmp >> 31) & 1) == 0) {
				H2K_TLB_ATOMIC_START;
				H2K_mem_tlb_write(tmp,0);
				H2K_TLB_ATOMIC_END;
			}
#endif
			H2K_isync();
		}
	}
}

void H2K_mem_tlb_invalidate_asid(u32_t asid) {

#if ARCHV >= 5
	asid <<=  (32 - PAGE_BITS);
	asm volatile
		(
		 " tlbinvasid(%0)\n"
		 :
		 : "r"(asid)
		 );

#else
	u32_t i;
	u64_t tmp,mask,check;
	mask = ((u64_t)(MAX_ASIDS - 1)) << (32+20);
	check = (((u64_t)(asid)) << (32+20)) & mask;
	for (i = ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1; i < MAX_TLB_ENTRIES; i++) {

		H2K_TLB_ATOMIC_START;
		tmp = H2K_mem_tlb_read(i);
		H2K_TLB_ATOMIC_END;

		if ((tmp & mask) == (check)) {
			H2K_TLB_ATOMIC_START;
			H2K_mem_tlb_write(i,0);
			H2K_TLB_ATOMIC_END;
		}
	}
#endif
	H2K_isync();
}
