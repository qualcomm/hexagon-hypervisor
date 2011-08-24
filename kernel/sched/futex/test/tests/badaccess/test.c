/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <h2.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

/*
 * This test checks the following functionality:
 * Futex wait on invalid locks
 * Futex resume on invalid locks
 * PI Mutex Lock on Invalid Locks
 * PI Mutex Unlock on Invalid Locks
 */

#define BADBASE 0xa0000000
#define PERMS(X) (0x90000000 | ((X) << 24))

void FAIL(const char *str)
{
	printf("FAIL: %s\n",str);
	exit(1);
}

void test_ptr_all(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success");
	if (h2_futex_wake(ptr,1) >= 0) FAIL("Wake returned success");
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_w(void *ptr)
{
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_rok(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success?");
	if (h2_futex_wake(ptr,1) > 0) FAIL("Wake woke thread?");
}

#if ARCHV <= 3
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	ret.xwr = perms >> 1;
	ret.guestonly = ~(perms & 1);
	ret.asid = asid;
	ret.size = size;
	ret.ppn = pa >> 12;
	ret.vpn = va >> 12;
	ret.valid = 1;
	return ret;
}
#else
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	pa >>= (12+size);
	pa = 1 + (pa << 1);
	pa <<= size;
	ret.ppd = pa;
	ret.vpn = va >> 12;
	ret.xwru = perms;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}
#endif

void touser()
{
	asm volatile (
	" r0 = ssr \n"
	" r0 = setbit(r0,#16) \n"
	" r0 = setbit(r0,#17) \n"
#if ARCHV >= 3
	" r0 = clrbit(r0,#19) \n"
#else
	" r0 = clrbit(r0,#13) \n"
#endif
	" ssr = r0 \n"
	" r0.h = #hi(1f) \n"
	" r0.l = #lo(1f) \n"
	" elr = r0 \n"
	" rte \n"
	"1: nop \n"
	: : : "r0");
}

int main() 
{
	int i;
	void *a;
	u32_t asid;
	H2K_mem_tlbfmt_t trans;
	h2_init(0);

	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));

	/* Bad address, OK memory */
	a = (void *)((u32_t)main | 1); test_ptr_all(a);
	a = (void *)((u32_t)main | 2); test_ptr_all(a);
	a = (void *)((u32_t)main | 3); test_ptr_all(a);

	/* Bad memory: not in TLB */
	a = (void *)(BADBASE | 0); test_ptr_all(a);
	a = (void *)(BADBASE | 1); test_ptr_all(a);
	a = (void *)(BADBASE | 2); test_ptr_all(a);
	a = (void *)(BADBASE | 3); test_ptr_all(a);

	/* Insufficient permissions */

	/* Note that I start out in monitor mode, so just go screw with the TLB directly */
	/* Generate all permissions */
	for (i = 0; i < 16; i++) {
		trans = make_entry(0x90000000 + (i << 24),0x0,6,i,asid);
		H2K_mem_tlb_write(32+i,trans.raw);
	}

	/* Check insufficient permissions: no R+W */
	a = (void *)(PERMS(0)); test_ptr_all(a);
	a = (void *)(PERMS(1)); test_ptr_all(a);
	a = (void *)(PERMS(2)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(3)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(4)); test_ptr_all(a);
	a = (void *)(PERMS(5)); test_ptr_all(a);
	a = (void *)(PERMS(8)); test_ptr_all(a);
	a = (void *)(PERMS(9)); test_ptr_all(a);
	a = (void *)(PERMS(10)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(11)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(12)); test_ptr_all(a);
	a = (void *)(PERMS(13)); test_ptr_all(a);

	touser();

	/* Check insufficient permissions: no R+W+U */
	a = (void *)(PERMS(0)); test_ptr_all(a);
	a = (void *)(PERMS(1)); test_ptr_all(a);
	a = (void *)(PERMS(2)); test_ptr_all(a);
	a = (void *)(PERMS(3)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(4)); test_ptr_all(a);
	a = (void *)(PERMS(5)); test_ptr_all(a);
	a = (void *)(PERMS(6)); test_ptr_all(a);
	a = (void *)(PERMS(8)); test_ptr_all(a);
	a = (void *)(PERMS(9)); test_ptr_all(a);
	a = (void *)(PERMS(10)); test_ptr_all(a);
	a = (void *)(PERMS(11)); test_ptr_w(a); test_ptr_rok(a);
	a = (void *)(PERMS(12)); test_ptr_all(a);
	a = (void *)(PERMS(13)); test_ptr_all(a);
	a = (void *)(PERMS(14)); test_ptr_all(a);

	puts("TEST PASSED");
	return 0;
}

