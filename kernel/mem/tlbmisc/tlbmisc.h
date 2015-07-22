/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBMISC_H
#define H2K_TLBMISC_H 1

#include <c_std.h>
#include <context.h>

#if ARCHV <= 3
static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)
{
	u32_t ret;
	asm (
	" tlbhi = %1\n"
	" tlbp\n"
	" %0 = tlbidx\n"
	: "=r"(ret)
	: "r"((asid << (32 - PAGE_BITS)) | (va >> PAGE_BITS)));
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
	: "r"(index),"r"(entry) : "memory");
}
#else

static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)
{
	u32_t ret;
	asm (
	" %0 = tlbp(%1);\n"
	: "=r"(ret)
	: "r"((asid << (32 - PAGE_BITS)) | (va >> PAGE_BITS)));
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
	: "r"(index),"r"(entry) : "memory");
}

#if ARCHV >= 5

static inline int H2K_mem_tlboc(u64_t entry)
{
	int ret;
	asm volatile(" %0 = tlboc(%1) " : "=r"(ret) : "r"(entry));
	return ret;
}

#endif

#endif

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me) IN_SECTION(".text.mem.tlb");
void H2K_mem_tlb_invalidate_asid(u32_t asid) IN_SECTION(".text.mem.tlb");

#endif

