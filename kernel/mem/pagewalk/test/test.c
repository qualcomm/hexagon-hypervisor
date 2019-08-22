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

int TH_translate_idx = 0;
int TH_expected_translates = 0;
long TH_expected_translate_pn[4];
u32_t TH_final_pn;

H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	if (TH_translate_idx >= TH_expected_translates) FAIL("too many translates");
	if (info.raw != 0xdeadbeefcafebabeULL) FAIL("bad info arg");
	if (TH_translate_idx != TH_expected_translates-1) {
		if (TH_expected_translate_pn[TH_translate_idx] != in.pn) FAIL("translate bad pn");
	} else {
		TH_final_pn = in.pn;
	}
	TH_translate_idx++;
	return in;
}

H2K_thread_context a;
H2K_vmblock_t myvmblock;
H2K_asid_entry_t info = {
	.ptb = 0,
	.fields = {
		.count = 1,
		.vmid = 2,
		.type = H2K_ASID_TRANS_TYPE_LINEAR,
		.log_maxhops = 2,
		.extra = 0,
	},
};

u32_t l1pt[1024] __attribute__((aligned(4096)));

u32_t l2pt_1MB[4] __attribute__((aligned(4096)));
u32_t l2pt_256KB[16] __attribute__((aligned(4096)));
u32_t l2pt_64KB[64] __attribute__((aligned(4096)));
u32_t l2pt_16KB[256] __attribute__((aligned(4096)));
u32_t l2pt_4KB[1024] __attribute__((aligned(4096)));

long l2addrs[] = {
	(long)l2pt_4KB,
	(long)l2pt_16KB,
	(long)l2pt_64KB,
	(long)l2pt_256KB,
	(long)l2pt_1MB,
};

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
	ret &= 0xffffffe7;
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
	H2K_thread_context_clear(&a);
	info.ptb = (u32_t)l1pt;
}

static inline H2K_pte_t pte_from_trans(H2K_translation_t trans)
{
	H2K_pte_t pte;
	pte.raw = 0;
	pte.s = trans.size;
	pte.xwr = trans.xwru >> 1;
	pte.u = trans.xwru & 1;
	pte.ppn = trans.pn & (-1 << (trans.size*2));
	pte.ccc = trans.cccc;
	return pte;
}

void test_all_firstpage()
{
	u32_t i;
	H2K_translation_t trans;
	H2K_pte_t result;
	for (i = 0; i < 4096; i++) {
		trans = H2K_translate_default(i);
		trans = H2K_pagewalk_translate(trans,info);
		result = pte_from_trans(trans);
		if (result.raw != l2pt_4KB[0]) {
			printf("i: %d result.raw: 0x%08x l2pt_4KB[0]: 0x%08x\n",i,result.raw,l2pt_4KB[0]);
			FAIL("firstpage addr: wrong translation");
		}
	}
}

void check_result(H2K_translation_t trans, u32_t addr, H2K_thread_context *thread)
{
	u32_t l1_idx = addr >> 22;
	u32_t l2_idx = (addr >> 12) & 0x03ff;
	u32_t size = (l1_idx >> 1) & 0x7;
	H2K_pte_t pte = pte_from_trans(trans);
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
			if ((pte.raw) != (pte_bits(l2_idx) | 1)) printf("addr: %08x trans: %016llx pte: %08x expected: %08x\n",addr,trans.raw,pte.raw,pte_bits(l2_idx)|1);
			if ((pte.raw) != (pte_bits(l2_idx) | 1)) FAIL("val fail/16K");
			break;
		case 0:
			if ((pte.s) != 0) FAIL("4K xlat fail: size");
			if ((pte.raw) != (pte_bits(l2_idx) | 0)) FAIL("val fail/4K");
			break;
	}
}

void test_all_pages()
{
	u32_t i;
	u32_t addr;
	H2K_translation_t trans;
	for (i = 0; i < 32*1024; i++) {
	        addr = i<<12;
		TH_expected_translates = 0;
		trans = H2K_translate_default(addr);
		trans = H2K_pagewalk_translate(trans,info);
		check_result(trans,addr,&a);
	}
}

void test_all_pages_stage2()
{
	u32_t i;
	u32_t addr;
	u32_t pgsize;
	H2K_translation_t trans;
	myvmblock.guestmap.raw = 0xdeadbeefcafebabeULL;
	TH_expected_translate_pn[0] = ((long)l1pt) >> 12;
	for (i = 0; i < 32*1024; i++) {
	        addr = i<<12;
		TH_translate_idx = 0;
		TH_expected_translates = 3;
		pgsize = (addr >> 23) & 7;
		if (pgsize == 7) pgsize = 6;
		if (pgsize >= 5) {
			TH_expected_translates = 2;
		} else {
			TH_expected_translate_pn[1] = l2addrs[pgsize] >> 12;
		};
		trans = H2K_translate_default(addr);
		trans = H2K_pagewalk_translate(trans,info);
		check_result(trans,addr,&a);
		if (TH_final_pn != trans.pn) FAIL("bad final translate");
		if (TH_expected_translates-1 > TH_translate_idx) FAIL("not enough translates");
	}
}

int main()
{
	/* Set up KGP correctly for direct calls */
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_gp->phys_offset = 0;
	H2K_gp->vmblocks[2] = &myvmblock;
	myvmblock.guestmap.raw = 0;
	
	setup();
	test_all_firstpage();
	puts("So far, so good...");
	test_all_pages();
	test_all_pages_stage2();
	/* FIXME: test fails from translate during walk / last translate */
	puts("TEST PASSED");
	return 0;
}

