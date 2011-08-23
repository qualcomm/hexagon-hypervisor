/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <globals.h>
#include <safemem.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <checker_tlb_locked.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a,b;
H2K_thread_context *TH_to;
H2K_thread_context *TH_from;

#if ARCHV <= 3
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	ret.xwr = perms >> 1;
	ret.guestonly = ~(perms & 1);
	ret.asid = 0x12;
	ret.size = size;
	ret.ppn = pa >> 12;
	ret.vpn = va >> 12;
	ret.valid = 1;
	return ret;
}
#else
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	pa >>= (12+size);
	pa = 1 + (pa << 1);
	pa <<= size;
	ret.ppd = pa;
	ret.vpn = va >> 12;
	ret.xwru = perms;
	ret.asid = 0x12;
	ret.valid = 1;
	return ret;
}

#endif

int main()
{
	pa_t pa;
	int i,j;
	H2K_mem_tlbfmt_t trans;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	a.ssr_asid = 0x12;
	if (H2K_safemem_check_and_lock((void *)0x1,0,&pa,&a)) FAIL("Misaligned pointer success");
	if (H2K_safemem_check_and_lock((void *)0x2,0,&pa,&a)) FAIL("Misaligned pointer success");
	if (H2K_safemem_check_and_lock((void *)0x3,0,&pa,&a)) FAIL("Misaligned pointer success");
	if (H2K_safemem_check_and_lock((void *)0x81000000,0,&pa,&a)) FAIL("Wayward Pointer Success");
	checker_tlb_unlocked();
	for (i = 0; i < 16; i++) {
		trans = make_entry(0x90000000 + (i << 24),0x0,6,i);
		H2K_mem_tlb_write(32+i,trans.raw);
	}
	a.ssr_guest = 0;
	for (i = 1; i < 8; i++) {
		if (H2K_safemem_check_and_lock((void *)0x90000000,i,&pa,&a)) {
			FAIL("Shouldn't have perms");
		}
		checker_tlb_unlocked();
	}
	a.ssr_guest = 1;
	for (i = 1; i < 8; i++) {
		if (H2K_safemem_check_and_lock((void *)0x90000000,i,&pa,&a)) {
			FAIL("Shouldn't have perms");
		}
		checker_tlb_unlocked();
	}
	a.ssr_guest = 0;
	for (i = 1; i < 8; i++) {
		for (j = 0; j < 16; j += 2) {
			if (H2K_safemem_check_and_lock((void *)0x90000000 + (j << 24),i,&pa,&a)) {
				FAIL("Shouldn't have guest perms");
			}
			checker_tlb_unlocked();
		}
	}

	for (i = 1; i < 8; i++) {
		for (j = 1; j < 16; j += 2) {
			if (((j >> 1) & i) == i) continue;
			if (H2K_safemem_check_and_lock((void *)0x90000000 + (j << 24),i,&pa,&a)) {
				FAIL("Shouldn't have perms");
			}
			checker_tlb_unlocked();
		}
	}

	for (i = 1; i < 8; i++) {
		for (j = 1; j < 16; j += 2) {
			if (((j >> 1) & i) != i) continue;
			if (!H2K_safemem_check_and_lock((void *)0x90000000 + (j << 24),i,&pa,&a)) {
				printf("j=%x i=%x\n",j,i);
				FAIL("Should have perms");
			}
			checker_tlb_locked();
			if (pa != 0) {
				printf("pa=%llx\n",(u64_t)pa);
				FAIL("Wrong PA");
			}
			H2K_safemem_unlock();
			checker_tlb_unlocked();
		}
	}

	a.ssr_guest = 1;

	for (i = 1; i < 8; i++) {
		for (j = 0; j < 16; j ++) {
			if (((j >> 1) & i) != i) continue;
			if (!H2K_safemem_check_and_lock((void *)0x90000000 + (j << 24),i,&pa,&a)) {
				printf("j=%x i=%x\n",j,i);
				FAIL("Should have perms");
			}
			checker_tlb_locked();
			if (pa != 0) {
				printf("pa=%llx\n",(u64_t)pa);
				FAIL("Wrong PA");
			}
			H2K_safemem_unlock();
			checker_tlb_unlocked();
		}
	}

	puts("TEST PASSED");
	return 0;
}

