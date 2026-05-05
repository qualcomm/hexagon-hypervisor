/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <tlb.h>
#include <fatal.h>
#include <globals.h>
#include <cache.h>
#include <hw.h>
#include <tlbmisc.h>
#include <symbols.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
// H2K_kg_t H2K_kg;

#define TLBCONST 0x0001234500000000ULL

static inline unsigned long long int TH_tlb_read(int index)
{
	unsigned long long int ret;
	asm volatile (" %0 = tlbr(%1) " : "=r"(ret) : "r"(index) : "memory");
	return ret;
}

static inline void TH_tlb_write(unsigned long long int entry, int index)
{
	asm volatile (" tlbw(%0,%1) " : : "r"(entry),"r"(index) : "memory");
}

void alloc_until_fail(int last_tlb)
{
	int count = 0;
	int index;
	while ((index = (int)H2K_tlb_tlbop(TLBOP_TLBALLOC,0,count | TLBCONST,&a)) >= 0) {
		if (TH_tlb_read(index) != (count|TLBCONST)) FAIL("readback");
		count++;
	}
	if (count != last_tlb - 63) FAIL("count");
	if (H2K_gp->pinned_tlb_mask != ~0ULL) FAIL("mask");
	if (H2K_gp->last_tlb_index != 63) FAIL("last index");
}

void free_all_0(int last_tlb)
{
	int i;
	printf("freeing...\n");
	for (i = 64; i <= last_tlb; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i-64)) & 1) FAIL("tlbmask bit");
		if (H2K_gp->last_tlb_index != i) {
			printf("%d vs i=%d\n",H2K_gp->last_tlb_index,i);
			FAIL("last entry");
		}
		if (TH_tlb_read(i) != 0) FAIL("free tlbr");
	}
}

void free_all_1(int last_tlb)
{
	int i;
	for (i = 65; i <= last_tlb; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i-64)) & 1) FAIL("free1 tlbmask bit");
		if (H2K_gp->last_tlb_index != 63) FAIL("free1 last entry");
		if (TH_tlb_read(i) != 0) FAIL("free1 tlbr");
	}
	H2K_tlb_tlbop(TLBOP_TLBFREE,64,0,&a);
	if ((H2K_gp->pinned_tlb_mask >> (64-64)) & 1) FAIL("free1 tlbmask bit/last");
	if (H2K_gp->last_tlb_index != last_tlb) FAIL("free1 last entry/last");
	if (TH_tlb_read(64) != 0) FAIL("free1 tlbr/last");
}
#define VALID_VPN 0x42000
#define VALID_VA (VALID_VPN<<12)
#define VALID_TLB_ENTRY ((((unsigned long long int)VALID_VPN)<<32) | 0x80000000feedfaceULL)
#define ASID 0x12
#define ASID_ENTRY (((unsigned long long int)ASID)<<(32+20))

void test_readwriteprobe(int last_tlb)
{
	int i;
	a.ssr_asid = ASID;
	for (i = 32; i <= last_tlb; i++) {
		if (H2K_tlb_tlbop(TLBOP_TLBWRITE,i,VALID_TLB_ENTRY,&a) != 0) FAIL("tlbw");
		if (H2K_tlb_tlbop(TLBOP_TLBQUERY,VALID_VA,0,&a) != i) FAIL("tlbp");
		if (H2K_tlb_tlbop(TLBOP_TLBREAD,i,0,&a) != (VALID_TLB_ENTRY | ASID_ENTRY)) FAIL("tlbr");
		if (H2K_tlb_tlbop(TLBOP_TLBWRITE,i,0,&a) != 0) FAIL("tlbw/2");
		if ((int)H2K_tlb_tlbop(TLBOP_TLBQUERY,VALID_VA,0,&a) >= 0) FAIL("tlbp/2");
		if (H2K_tlb_tlbop(TLBOP_TLBREAD,i,0,&a) != ASID_ENTRY) FAIL("tlbr/2");
	}
}

void random_test()
{

}

int main()
{
#if ARCHV > 4
	int i;
	u32_t npages   = (u32_t)&H2K_KERNEL_NPAGES;
	u32_t tlb_size = H2K_TLB_SIZE;
	/* mirrors boot.ref.S: LAST_TLB = TLB_SIZE - NPAGES - 2 - 1
	 * (-2 for angel + device pages, -1 for the sub-then-decrement step) */
	int   last_tlb = (int)(tlb_size - npages - 3);
	u64_t expected_init_mask = (~0ULL) << ((last_tlb + 1) & 0x3F);

	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	printf("Hello!\n");
	H2K_kg_init(0, 0, 0, last_tlb, tlb_size, 0, 0, 0);
	printf("initted!\n");
	if (H2K_gp->pinned_tlb_mask != expected_init_mask) {
		FAIL("mask setup");
	}
	TH_tlb_write(0x01234567cafebabeULL, last_tlb + 2);
	if (TH_tlb_read(last_tlb + 2) != 0x01234567cafebabeULL) FAIL("TH");
	TH_tlb_write(0x01234567deadbeefULL, last_tlb + 1);
	if (TH_tlb_read(last_tlb + 1) != 0x01234567deadbeefULL) FAIL("TH");
	printf("Allocing!\n");
	alloc_until_fail(last_tlb);
	free_all_0(last_tlb);
	alloc_until_fail(last_tlb);
	free_all_1(last_tlb);
	printf("readwriteprobe...\n");
	test_readwriteprobe(last_tlb);
	printf("Random Testing!\n");
	for (i = 0; i < 1000; i++) {
		random_test();
	}
#endif
	puts("TEST PASSED\n");
	return 0;
}
