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

#define TRANS_ENTRY(VPN,PPN,SIZE,CCCC,PERMS) \
	( (((unsigned long long int)((SIZE) & 0xF)) << (32+20)) \
	| (((unsigned long long int)((VPN) & 0xFFFFF)) << (32)) \
	| (((unsigned long long int)((PERMS) & 0xF)) << (28)) \
	| (((unsigned long long int)((CCCC) & 0xF)) << (24)) \
	| (((unsigned long long int)((PPN) & 0xFFFFFF)) << (0)))

unsigned long long int transtab[] = {
	TRANS_ENTRY(0x01000,0x001000,6,0x7,0xf),
	TRANS_ENTRY(0x02000,0x002000,6,0x7,0xf),
	TRANS_ENTRY(0x03000,0x003000,6,0x7,0xf),
	TRANS_ENTRY(0x04000,0x004000,6,0x7,0xf),
	TRANS_ENTRY(0x05000,0x005000,6,0x7,0xf),
	TRANS_ENTRY(0x06000,0x006000,6,0x7,0xf),
	TRANS_ENTRY(0x90000,0x900000,6,0x7,0x0),
	TRANS_ENTRY(0x91000,0x900000,6,0x7,0x1),
	TRANS_ENTRY(0x92000,0x900000,6,0x7,0x2),
	TRANS_ENTRY(0x93000,0x900000,6,0x7,0x3),
	TRANS_ENTRY(0x94000,0x900000,6,0x7,0x4),
	TRANS_ENTRY(0x95000,0x900000,6,0x7,0x5),
	TRANS_ENTRY(0x96000,0x900000,6,0x7,0x6),
	TRANS_ENTRY(0x97000,0x900000,6,0x7,0x7),
	TRANS_ENTRY(0x98000,0x900000,6,0x7,0x8),
	TRANS_ENTRY(0x99000,0x900000,6,0x7,0x9),
	TRANS_ENTRY(0x9a000,0x900000,6,0x7,0xa),
	TRANS_ENTRY(0x9b000,0x900000,6,0x7,0xb),
	TRANS_ENTRY(0x9c000,0x900000,6,0x7,0xc),
	TRANS_ENTRY(0x9d000,0x900000,6,0x7,0xd),
	TRANS_ENTRY(0x9e000,0x900000,6,0x7,0xe),
	TRANS_ENTRY(0x9f000,0x900000,6,0x7,0xf),
	0,
};

void FAIL(const char *str)
{
	printf("FAIL: %s\n",str);
	exit(1);
}

void test_ptr_all(void *ptr)
{
	if (h2_futex_wait(ptr,1) >= 0) FAIL("all/Wait returned success");
	if (h2_futex_wake(ptr,1) >= 0) {
		printf("ptr=%p\n",ptr);
		FAIL("all/Wake returned success");
	}
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("all/Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("all/Unlock returned success");
}

void test_ptr_w(void *ptr)
{
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("w/Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("w/Unlock returned success");
}

void test_ptr_rok(void *ptr)
{
	if (h2_futex_wait(ptr,1) >= 0) FAIL("r/Wait returned success?");
	if (h2_futex_wake(ptr,1) > 0) FAIL("r/Wake woke thread?");
}

#if 0
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
#endif

void touser()
{
	asm volatile (
	" g2 = r29; // gosp = sp "
	" r29 = ##1f; "
	" g0 = r29; // gelr = future address "
	" r29 = ##0x80000000; // UM only "
	" g1 = r29; "
	" r29 = g2; // restore sp "
	" trap1(#1); // VMRTE "
	" 1: nop; "
	:::"memory");
}

int main() 
{
	void *a;

	puts("Starting\n");
	h2_handle_errors(1);

	h2_vmtrap_newmap(transtab,H2K_ASID_TRANS_TYPE_LINEAR,0);
	puts("mapped\n");
	/* Bad address, OK memory */
	a = (void *)((u32_t)main | 1); test_ptr_all(a);
	a = (void *)((u32_t)main | 2); test_ptr_all(a);
	a = (void *)((u32_t)main | 3); test_ptr_all(a);

	/* Bad memory: not in TLB */
	a = (void *)(BADBASE | 0); test_ptr_all(a);
	a = (void *)(BADBASE | 1); test_ptr_all(a);
	a = (void *)(BADBASE | 2); test_ptr_all(a);
	a = (void *)(BADBASE | 3); test_ptr_all(a);

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

	puts("Switching to user...\n");
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

