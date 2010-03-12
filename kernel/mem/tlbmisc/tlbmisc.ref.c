/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
#include <max.h>

#if __QDSP6_ARCH__ <= 3
static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)
{
	u32_t ret;
	asm (
	" tlbhi = %1\n"
	" tlbp\n"
	" %0 = tlbidx\n"
	: "=r"(ret)
	: "r"((asid<<20)|(va >> 12)));
	return ret;
}

static inline u64_t H2K_mem_tlb_read(u32_t index)
{
	u64_t ret;
	asm (
	" tlbidx = %1\n"
	" tlbr\n"
	" %H0 = tlbhi\n"
	" %L0 = tlblo\n"
	: "=r"(ret)
	: "r"(index));
	return ret;
}

static inline void H2K_mem_tlb_write(u32_t index, u64_t entry)
{
	asm volatile (
	" tlbidx = %0\n"
	" tlbhi = %H1\n"
	" tlblo = %L1\n"
	" tlbw\n"
	:
	: "r"(index),"r"(entry));
}
#else

#warning doublecheck for v4

static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)
{
	u32_t ret;
	asm (
	" %0 = tlbp(%1);\n"
	: "=r"(ret)
	: "r"((asid<<20)|(va >> 12)));
	return ret;
}

static inline u64_t H2K_mem_tlb_read(u32_t index)
{
	u64_t ret;
	asm (
	" %0 = tlbr(%1);\n"
	: "=r"(ret)
	: "r"(index));
	return ret;
}

static inline void H2K_mem_tlb_write(u32_t index, u64_t entry)
{
	asm volatile (
	" tlbw(%1,%0)\n"
	:
	: "r"(index),"r"(entry));
}

#endif

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)
{
	u32_t tmp;
	tmp = H2K_mem_tlb_probe(va,asid);
	if (((tmp >> 31) & 1) == 0) {
		H2K_mem_tlb_write(tmp,0);
	}
	/* For V3 and earlier, also need to probe fake guest bit */
}

void H2K_mem_tlb_invalidate_asid(u32_t asid)
{
	u32_t i;
	u64_t tmp,mask,check;
	mask = ((u64_t)(MAX_TLB_ENTRIES-1)) << (32+20);
	check = (((u64_t)(asid)) << (32+20)) & mask;
	for (i = TLB_FIRST_REPLACEABLE_ENTRY; i < MAX_TLB_ENTRIES; i++) {
		tmp = H2K_mem_tlb_read(i);
		if ((tmp & mask) == (check)) {
			H2K_mem_tlb_write(i,0);
		}
	}
}

