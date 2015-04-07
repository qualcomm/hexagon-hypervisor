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

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
// H2K_kg_t H2K_kg;

#define TLBCONST 0x0102030400000000ULL

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

void alloc_until_fail()
{
	int count = 0;
	int index;
	while ((index = (int)H2K_tlb_tlbop(TLBOP_TLBALLOC,0,count | TLBCONST,&a)) >= 0) {
		if (TH_tlb_read(index) != (count|TLBCONST)) FAIL("readback");
		count++;
	}
	if (count != 62) FAIL("count");
	if (H2K_gp->pinned_tlb_mask != ~0ULL) FAIL("mask");
	if (H2K_gp->last_tlb_index != 63) FAIL("last index");
}

void free_all_0()
{
	int i;
	printf("freeing...\n");
	for (i = 64; i <= 125; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i-64)) & 1) FAIL("tlbmask bit");
		if (H2K_gp->last_tlb_index != i) {
			printf("%d vs i=%d\n",H2K_gp->last_tlb_index,i);
			FAIL("last entry");
		}
		if (TH_tlb_read(i) != 0) FAIL("free tlbr");
	}
}

void free_all_1()
{
	int i;
	for (i = 65; i <= 125; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i-64)) & 1) FAIL("free1 tlbmask bit");
		if (H2K_gp->last_tlb_index != 63) FAIL("free1 last entry");
		if (TH_tlb_read(i) != 0) FAIL("free1 tlbr");
	}
	H2K_tlb_tlbop(TLBOP_TLBFREE,64,0,&a);
	if ((H2K_gp->pinned_tlb_mask >> (64-64)) & 1) FAIL("free1 tlbmask bit/last");
	if (H2K_gp->last_tlb_index != 125) FAIL("free1 last entry/last");
	if (TH_tlb_read(64) != 0) FAIL("free1 tlbr/last");
}
#define VALID_VPN 0x42000
#define VALID_VA (VALID_VPN<<12)
#define VALID_TLB_ENTRY ((((unsigned long long int)VALID_VPN)<<32) | 0x80000000feedfaceULL)
#define ASID 0x12
#define ASID_ENTRY (((unsigned long long int)ASID)<<(32+20))

void test_readwriteprobe()
{
	int i;
	int idx;
	a.ssr_asid = ASID;
	for (i = 32; i < 96; i++) {
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
	int i;
#if ARCHV > 4
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	printf("Hello!\n");
	H2K_kg_init(0,0,125);
	printf("initted!\n");
	if (H2K_gp->pinned_tlb_mask != 0xC000000000000000ULL) {
		FAIL("mask setup");
	}
	TH_tlb_write(0x01234567cafebabeULL,127);
	if (TH_tlb_read(127)!=0x01234567cafebabeULL) FAIL("TH");
	TH_tlb_write(0x01234567deadbeefULL,126);
	if (TH_tlb_read(126)!=0x01234567deadbeefULL) FAIL("TH");
	printf("Allocing!\n");
	alloc_until_fail();
	free_all_0();
	alloc_until_fail();
	free_all_1();
	printf("readwriteprobe...\n");
	test_readwriteprobe();
	printf("Random Testing!\n");
	for (i = 0; i < 1000; i++) {
		random_test();
	}
#endif
	puts("TEST PASSED\n");
	return 0;
}

