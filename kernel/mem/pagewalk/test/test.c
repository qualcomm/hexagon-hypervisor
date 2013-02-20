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
#include <thread.h>
#include <globals.h>

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

#define L1_16MB  0x00000006
#define L1_4MB   0x00000005
#define L1_4KB   0x00000000
#define L1_16KB  0x00000001
#define L1_64KB  0x00000002
#define L1_256KB 0x00000003
#define L1_1MB   0x00000004

#define L2_4KB   0x00000000
#define L2_16KB  0x00000001
#define L2_64KB  0x00000002
#define L2_256KB 0x00000003
#define L2_1MB   0x00000004

static inline unsigned int pte_bits(unsigned int i)
{
	u32_t ret;
	ret = ((i & 0x0ff)<<4);	/* XWR / CC / UT bits */
	ret |= ((i & 0xff0) << 20);
	return ret;
}

/* 
 * Set Up Page Tables
 * Make all sizes of L2 tables
 * Make L1 page table with various sizes
 */
void setup()
{
	u32_t pgsize;
	u32_t i;
	u32_t l1_4KB_entry   = (L1_4KB   | (((u32_t)l2pt_4KB)  ));
	u32_t l1_16KB_entry  = (L1_16KB  | (((u32_t)l2pt_16KB) ));
	u32_t l1_64KB_entry  = (L1_64KB  | (((u32_t)l2pt_64KB) ));
	u32_t l1_256KB_entry = (L1_256KB | (((u32_t)l2pt_256KB)));
	u32_t l1_1MB_entry   = (L1_1MB   | (((u32_t)l2pt_1MB)  ));
	for (i = 0; i < 4;    i++) {
		l2pt_1MB[i]   = L2_1MB   | pte_bits(i);
	}
	for (i = 0; i < 16;   i++) {
		l2pt_256KB[i] = L2_256KB | pte_bits(i);
	}
	for (i = 0; i < 64;   i++) {
		l2pt_64KB[i]  = L2_64KB  | pte_bits(i);
	}
	for (i = 0; i < 256;  i++) {
		l2pt_16KB[i]  = L2_16KB  | pte_bits(i);
	}
	for (i = 0; i < 1024; i++) {
		l2pt_4KB[i]   = L2_4KB   | pte_bits(i);
	}
	for (i = 0; i < 1024; i++) {
		pgsize = (i>>1)&7;
		switch (pgsize) {
			case 0: l1pt[i+0] = l1_4KB_entry;    break;
			case 1: l1pt[i+0] = l1_16KB_entry;   break;
			case 2: l1pt[i+0] = l1_64KB_entry;   break;
			case 3: l1pt[i+0] = l1_256KB_entry;  break;
			case 4: l1pt[i+0] = l1_1MB_entry;    break;
			case 5: l1pt[i+0] = L1_4MB  | pte_bits(i); break;
			case 6:
				l1pt[i+0] = L1_16MB | pte_bits(i);
				l1pt[i+1] = L1_16MB | pte_bits(i);
				l1pt[i+2] = L1_16MB | pte_bits(i);
				l1pt[i+3] = L1_16MB | pte_bits(i);
				i += 3;
				break;
			default: break;
		}
	}
	/* OK, set up ASID table... */
	H2K_asid_table_init();
	H2K_thread_context_clear(&a);
	a.ssr_asid = H2K_asid_table_inc(((u32_t)l1pt), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, NULL);
}

void test_all_firstpage()
{
	u32_t i;
	H2K_pte_t result;
	for (i = 0; i < 4096; i++) {
		result = H2K_mem_pagewalk(i,&a);
		if (result.raw != l2pt_4KB[0]) {
			printf("i: %d result.raw: 0x%08x l2pt_4KB[0]: 0x%08x\n",i,result.raw,l2pt_4KB[0]);
			FAIL("firstpage addr: wrong translation");
		}
	}
}

#define CHECK(A,OP,B) if (!((A) OP (B))) FAIL(#A #OP #B);
#if ARCHV <= 3
void check_tlbfmt(u32_t addr, u32_t size, u32_t expectval, H2K_thread_context *t)
{
	H2K_mem_tlbfmt_t tlb = H2K_mem_get_pagetable(addr,t);
	if (((expectval >> 5) & 7) == 0) {
		CHECK(tlb.raw,==,0);
		return;
	}
	CHECK(tlb.ppn>>12,==,((expectval >> 4)& 0xff));
	CHECK(tlb.size,==,tlb.size);
	CHECK(tlb.vpn,==,addr>>12);
	if (tlb.asid != t->ssr_asid) {
		printf("tlbasid: %x ssrasid: %x\n",tlb.asid,t->ssr_asid);
	}
	CHECK(tlb.asid,==,t->ssr_asid);
	CHECK(tlb.ccc,==,(expectval >> 2) & 0x7);
	CHECK(tlb.xwr,==,(expectval >> 5) & 0x7);
	CHECK(tlb.global,==,0);
}
#else
void check_tlbfmt(u32_t addr, u32_t size, u32_t expectval, H2K_thread_context *t)
{
	H2K_mem_tlbfmt_t tlb = H2K_mem_get_pagetable(addr,t);
	if (((expectval >> 5) & 7) == 0) {
		CHECK(tlb.raw,==,0);
		return;
	}
#if 0
	printf("tlb: 0x%016llx tlb.ppd>>1: 0x%016llx expectval: 0x%08x\n",
		(u64_t)tlb.raw,(u64_t)tlb.ppd>>1,expectval);
#endif
	CHECK(tlb.ppd>>13,==,((expectval >> 4)& 0xff));
	CHECK(tlb.vpn,==,addr>>12);
	CHECK(tlb.asid,==,t->ssr_asid);
	CHECK(tlb.cccc,==,(expectval >> 2) & 0x7);
	CHECK(tlb.xwru>>1,==,(expectval >> 5) & 0x7);
	CHECK(tlb.xwru&1,==,(expectval >>1) & 1);
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
			if ((pte.s) != 6) FAIL("16MB xlat fail: size");
			l2_idx = l1_idx & -4;
			if ((pte.raw) != (pte_bits(l2_idx) | 6)) FAIL("val fail/16M");
			break;
		case 5:
			if ((pte.s) != 5) FAIL("4MB xlat fail: size");
			l2_idx = l1_idx;
			if ((pte.raw) != (pte_bits(l2_idx) | 5)) FAIL("val fail/4M");
			break;
		case 4:
			if ((pte.s) != 4) FAIL("1MB xlat fail: size");
			l2_idx >>= 2*4;
			if ((pte.raw) != (pte_bits(l2_idx) | 4)) FAIL("val fail/1M");
			break;
		case 3:
			if ((pte.s) != 3) FAIL("256K xlat fail: size");
			l2_idx >>= 2*3;
			if ((pte.raw) != (pte_bits(l2_idx) | 3)) FAIL("val fail/256K");
			break;
		case 2:
			if ((pte.s) != 2) FAIL("64K xlat fail: size");
			l2_idx >>= 2*2;
			if ((pte.raw) != (pte_bits(l2_idx) | 2)) FAIL("val fail/64K");
			break;
		case 1:
			if ((pte.s) != 1) FAIL("16K xlat fail: size");
			l2_idx >>= 2*1;
			if ((pte.raw) != (pte_bits(l2_idx) | 1)) FAIL("val fail/16K");
			break;
		case 0:
			if ((pte.s) != 0) FAIL("4K xlat fail: size");
			if ((pte.raw) != (pte_bits(l2_idx) | 0)) FAIL("val fail/4K");
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
	/* Set up KGP correctly for direct calls */
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_gp->phys_offset = 0;
	
	setup();
	test_all_firstpage();
	puts("So far, so good...");
	test_all_pages();
	puts("TEST PASSED");
	return 0;
}

