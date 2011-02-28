/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <max.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <pagewalk.h>
#include <asid.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

u32_t l1pt[1024] __attribute__((aligned(4096)));

u32_t l2pt_1MB[4] __attribute__((aligned(16)));
u32_t l2pt_256KB[16] __attribute__((aligned(64)));
u32_t l2pt_64KB[64] __attribute__((aligned(256)));
u32_t l2pt_16KB[256] __attribute__((aligned(1024)));
u32_t l2pt_4KB[1024] __attribute__((aligned(4096)));

#define L1_16MB  0x00000040
#define L1_4MB   0x00000020
#define L1_4KB   0x00000010
#define L1_16KB  0x00000008
#define L1_64KB  0x00000004
#define L1_256KB 0x00000002
#define L1_1MB   0x00000001

#define L2_4KB   0x00000001
#define L2_16KB  0x00000002
#define L2_64KB  0x00000004
#define L2_256KB 0x00000008
#define L2_1MB   0x00000010

void setup()
{
	u32_t pgsize;
	u32_t i;
	u32_t l1_4KB_entry   = (L1_4KB   | (((u32_t)l2pt_4KB)  >>3));
	u32_t l1_16KB_entry  = (L1_16KB  | (((u32_t)l2pt_16KB) >>3));
	u32_t l1_64KB_entry  = (L1_64KB  | (((u32_t)l2pt_64KB) >>3));
	u32_t l1_256KB_entry = (L1_256KB | (((u32_t)l2pt_256KB)>>3));
	u32_t l1_1MB_entry   = (L1_1MB   | (((u32_t)l2pt_1MB)  >>3));
	for (i = 0; i < 4;    i++) {
		l2pt_1MB[i]   = L2_1MB   | i<<16;
	}
	for (i = 0; i < 16;   i++) {
		l2pt_256KB[i] = L2_256KB | i<<16;
	}
	for (i = 0; i < 64;   i++) {
		l2pt_64KB[i]  = L2_64KB  | i<<16;
	}
	for (i = 0; i < 256;  i++) {
		l2pt_16KB[i]  = L2_16KB  | i<<16;
	}
	for (i = 0; i < 1024; i++) {
		l2pt_4KB[i]   = L2_4KB   | i<<16;
	}
	for (i = 0; i < 1024; i++) {
		pgsize = (i>>1)&7;
		switch (pgsize) {
			case 0: l1pt[i+0] = l1_4KB_entry;    break;
			case 1: l1pt[i+0] = l1_16KB_entry;   break;
			case 2: l1pt[i+0] = l1_64KB_entry;   break;
			case 3: l1pt[i+0] = l1_256KB_entry;  break;
			case 4: l1pt[i+0] = l1_1MB_entry;    break;
			case 5: l1pt[i+0] = L1_4MB  | i<<16; break;
			case 6:
				l1pt[i+0] = L1_16MB | i<<16;
				l1pt[i+1] = L1_16MB | i<<16;
				l1pt[i+2] = L1_16MB | i<<16;
				l1pt[i+3] = L1_16MB | i<<16;
				i += 3;
				break;
			default: break;
		}
	}
	/* OK, set up ASID table... */
	H2K_asid_table_init();
	a.ssr_asid = H2K_asid_table_inc(((u32_t)l1pt)>>12);
}

void test_all_firstpage()
{
	u32_t i;
	H2K_pte_t result;
	for (i = 0; i < 4096; i++) {
		result = H2K_mem_pagewalk(i,&a);
		if (result.raw != l2pt_4KB[0]) FAIL("firstpage addr: wrong translation");
	}
}

#define CHECK(A,OP,B) if (!((A) OP (B))) FAIL(#A #OP #B);
#if __QDSP6_ARCH__ <= 3
void check_tlbfmt(u32_t addr, u32_t size, u32_t expectval, H2K_thread_context *t)
{
	H2K_mem_tlbfmt_t tlb = H2K_mem_translate_pagetable(addr,t);
	CHECK(tlb.ppn>>15,==,(expectval & 0x1f));
	CHECK(tlb.size,==,tlb.size);
	CHECK(tlb.vpn,==,addr>>12);
	CHECK(tlb.asid,==,t->ssr_asid);
	CHECK(tlb.ccc,==,(expectval >> 8) & 0x7);
	CHECK(tlb.xwr,==,(expectval >> 12) & 0x7);
	CHECK(tlb.global,==,0);
}
#else
void check_tlbfmt(u32_t addr, u32_t size, u32_t expectval, H2K_thread_context *t)
{
	H2K_mem_tlbfmt_t tlb = H2K_mem_translate_pagetable(addr,t);
	CHECK(tlb.ppd>>16,==,(expectval & 0xff));
	CHECK(tlb.vpn,==,addr>>12);
	CHECK(tlb.asid,==,t->ssr_asid);
	CHECK(tlb.cccc,==,(expectval >> 8) & 0xf);
	CHECK(tlb.xwru,==,(expectval >> 12) & 0xf);
	CHECK(tlb.global,==,0);
}
#endif

void check_result(H2K_pte_t pte, u32_t addr, H2K_thread_context *thread)
{
	u32_t l1_idx = addr >> 22;
	u32_t l2_idx = (addr >> 12) & 0x03ff;
	u32_t size = (l1_idx >> 1) & 0x7;
	switch (size) {
		case 7:
			size = 6;
			/* FALLTHROUGH */
		case 6:
			if ((pte.raw & 0x3f) != 0) FAIL("16MB xlat fail: size");
			if ((pte.raw & 0x40) == 0) FAIL("16MB xlat fail: size(2)");
			l2_idx = l1_idx & 0xfffc;
			if ((pte.raw >> 16) != (l2_idx)) FAIL("16MB xlat fail: val");
			break;
		case 5:
			if ((pte.raw & 0x1f) != 0) FAIL("4MB xlat fail: size");
			if ((pte.raw & 0x20) == 0) FAIL("4MB xlat fail: size(2)");
			l2_idx = l1_idx;
			if ((pte.raw >> 16) != (l2_idx)) FAIL("4MB xlat fail: val");
			break;
		case 4:
			if ((pte.raw & 0x0f) != 0) FAIL("1MB xlat fail: size");
			if ((pte.raw & 0x10) == 0) FAIL("1MB xlat fail: size(2)");
			l2_idx >>= 2*4;
			if ((pte.raw >> 16) != ((l2_idx) & 0x0003)) FAIL("1MB xlat fail: val");
			break;
		case 3:
			if ((pte.raw & 0x07) != 0) FAIL("256KB xlat fail: size");
			if ((pte.raw & 0x08) == 0) FAIL("256KB xlat fail: size(2)");
			l2_idx >>= 2*3;
			if ((pte.raw >> 16) != ((l2_idx) & 0x000f)) FAIL("256KB xlat fail: val");
			break;
		case 2:
			if ((pte.raw & 0x03) != 0) FAIL("64KB xlat fail: size");
			if ((pte.raw & 0x04) == 0) FAIL("64KB xlat fail: size(2)");
			l2_idx >>= 2*2;
			if ((pte.raw >> 16) != ((l2_idx) & 0x003f)) FAIL("64KB xlat fail: val");
			break;
		case 1:
			if ((pte.raw & 0x01) != 0) FAIL("16KB xlat fail: size");
			if ((pte.raw & 0x02) == 0) FAIL("16KB xlat fail: size(2)");
			l2_idx >>= 2*1;
			if ((pte.raw >> 16) != ((l2_idx) & 0x00ff)) FAIL("16KB xlat fail: val");
			break;
		case 0:
			if ((pte.raw & 0x00) != 0) FAIL("4KB xlat fail: size");
			if ((pte.raw & 0x01) == 0) FAIL("4KB xlat fail: size(2)");
			if ((pte.raw >> 16) != ((l2_idx) & 0x03ff)) FAIL("4KB xlat fail: val");
			break;
	}
	check_tlbfmt(addr,size,l2_idx,thread);
}

void test_all_pages()
{
	u32_t i;
	u32_t addr;
	H2K_pte_t result;
	for (i = 0; i < 32*1024; i++) {
	        addr = i<<12;
		result = H2K_mem_pagewalk(addr,&a);
		check_result(result,addr,&a);
	}
}

int main()
{
	setup();
	test_all_firstpage();
	puts("So far, so good...");
	test_all_pages();
	puts("TEST PASSED");
	return 0;
}

