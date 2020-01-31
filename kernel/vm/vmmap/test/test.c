/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <globals.h>
#include <tlbmisc.h>
#include <stlb.h>
#include <asid.h>
#include <vmmap.h>
#include <tmpmap.h>
void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
H2K_vmblock_t av;

u32_t TH_saw_fatal;

void H2K_fatal_thread()
{
	FAIL("Saw fatal");
}

H2K_kg_t H2K_kg;

u32_t TH_expected_stlb_invasid = 0;
u32_t TH_expected_tlb_invasid = 0;
u32_t TH_expected_stlb_inv_va = 0;
u32_t TH_expected_tlb_inv_va = 0;
u32_t TH_saw_stlb_invasid = 0;
u32_t TH_saw_tlb_invasid = 0;
u32_t TH_saw_stlb_inv_va = 0;
u32_t TH_saw_tlb_inv_va = 0;

u32_t TH_oldasid = 0;
u32_t TH_newasid = 0;

void H2K_mem_stlb_invalidate_asid_ext(u32_t asid)
{
	if (TH_expected_stlb_invasid == 0) FAIL("Unexpected invalidate stlb");
	TH_expected_stlb_invasid = 0;
	if (asid != TH_oldasid) FAIL("unexpected asid inv stlb");
	TH_saw_stlb_invasid = 1;
}

void H2K_mem_stlb_invalidate_va_ext(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me) {
	if (TH_expected_stlb_inv_va == 0) FAIL("Unexpected invalidate stlb va");
	TH_expected_stlb_inv_va = 0;
	if (asid != TH_oldasid) FAIL("unexpected asid inv stlb");
	TH_saw_stlb_inv_va = 1;
}

void H2K_mem_tlb_invalidate_asid_ext(u32_t asid)
{
	if (TH_expected_tlb_invasid == 0) FAIL("Unexpected invalidate tlb");
	TH_expected_tlb_invasid = 0;
	if (asid != TH_oldasid) FAIL("unexpected asid inv tlb");
	TH_saw_tlb_invasid = 1;
}

void H2K_mem_tlb_invalidate_va_ext(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me) {
	if (TH_expected_tlb_inv_va == 0) FAIL("Unexpected invalidate tlb va");
	TH_expected_tlb_inv_va = 0;
	if (asid != TH_oldasid) FAIL("unexpected asid inv tlb");
	TH_saw_tlb_inv_va = 1;
}

u32_t TH_expected_table_inc = 0;
u32_t TH_expected_table_dec = 0;
u32_t TH_saw_table_inc = 0;
u32_t TH_saw_table_dec = 0;

s32_t H2K_asid_table_inc(u32_t newptb, translation_type type, tlb_invalidate_flag flag, u32_t extra, H2K_vmblock_t *vmblock)
{
	if (TH_expected_table_inc == 0) FAIL("Unexpected inc");
	TH_expected_table_inc = 0;
	if (newptb != a.r00) FAIL("newptb arg");
	if (type != a.r01) FAIL("type arg");
	if (flag != a.r02) FAIL("flag arg");
	if (vmblock != &av) FAIL("vmblock");
	TH_saw_table_inc = 1;
	return TH_newasid;
}

void H2K_asid_table_dec(u32_t asid)
{
	if (TH_expected_table_dec == 0) FAIL("Unexpected inc");
	if (asid != TH_oldasid) FAIL("Wrong ASID");
	TH_saw_table_dec = 1;
}

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_kg.last_tlb_index = 0x7d;
	H2K_kg.tlb_size = 0x80;
	u32_t asid;
	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));
	a.vmblock = &av;

	/* CLRMAP */
	TH_oldasid = a.ssr_asid = 0x12;
	a.r00 = 0x1000;
	a.r01 = 0;
	H2K_vmtrap_clrmap(&a);
	if (a.r01 != 0) {
		H2K_mem_stlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
		H2K_mem_tlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
	}
	if (TH_saw_stlb_invasid || TH_saw_tlb_invasid) FAIL("saw invalidate on 0 count");
	if (a.r00 != 0) FAIL("clrmap ret");

	TH_oldasid = a.ssr_asid = 0x12;
	a.r00 = 0x1000;
	a.r01 = 0x10000;
	TH_expected_tlb_inv_va = TH_expected_stlb_inv_va = 1;
	H2K_vmtrap_clrmap(&a);
	if (a.r01 != 0) {
		H2K_mem_stlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
		H2K_mem_tlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
	}
	if (!(TH_saw_stlb_inv_va && TH_saw_tlb_inv_va)) FAIL("no invalidate");
	if (a.r00 != 0) FAIL("clrmap ret");
	TH_saw_stlb_inv_va = TH_saw_tlb_inv_va = 0;

#if 0
	TH_oldasid = a.ssr_asid = asid;
	a.r00 = H2K_tmpmap_add_and_lock((pa_t)H2K_LINK_ADDR, UNCACHED);
	a.r01 = 0x1;
	TH_expected_tlb_inv_va = TH_expected_stlb_inv_va = 1;
	H2K_vmtrap_clrmap(&a);
	if (a.r01 != 0) {
		H2K_mem_stlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
		H2K_mem_tlb_invalidate_va_ext(a.r00, a.r01, a.ssr_asid, &a);
	}
	H2K_tmpmap_remove_and_unlock();
	if (!(TH_saw_stlb_inv_va && TH_saw_tlb_inv_va)) FAIL("no invalidate on 1 count");
	if (a.r00 != 0) FAIL("clrmap ret");
	TH_saw_stlb_inv_va = TH_saw_tlb_inv_va = 0;
#endif

	/* NEWMAP */

	TH_oldasid = a.ssr_asid = 0x12;
	a.r00 = 0x10000;
	a.r01 = 0x12345;
	a.r02 = 0;
	H2K_vmtrap_newmap(&a);
	if (a.r00 != -1) FAIL("bad trans type didn't fail");

	TH_oldasid = a.ssr_asid = 0x12;
	a.r00 = 0x10000;
	a.r01 = 0x0;
	a.r02 = 0;
	TH_expected_table_inc = 1;
	TH_newasid = -1;
	H2K_vmtrap_newmap(&a);
	if (a.r00 != -1) FAIL("bad asid return didn't fail");
	if (TH_saw_table_dec) FAIL("saw dec");
	if (!TH_saw_table_inc) FAIL("saw no inc");
	TH_saw_table_dec = TH_saw_table_inc = 0;

	TH_oldasid = a.ssr_asid = 0x12;
	a.r00 = 0x10000;
	a.r01 = 0x0;
	a.r02 = 0;
	TH_expected_table_inc = 1;
	TH_expected_table_dec = 1;
	TH_newasid = 2;
	H2K_vmtrap_newmap(&a);
	if (a.r00 != 0) FAIL("bad return");
	if (a.ssr_asid != TH_newasid) FAIL("asid");
	if (!TH_saw_table_inc || !TH_saw_table_dec) FAIL("no inc/dec");

	puts("TEST PASSED");
	return 0;
}

