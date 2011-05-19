/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stop.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <thread.h>
#include <hw.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_kernel_locked.h>
#include <globals.h>
#include <vm.h>

u32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me);
void H2K_interrupt_restore();

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;

u32_t TH_saw_check_sanity = 0;
H2K_thread_context *TH_me = NULL;

u64_t H2K_check_sanity_unlock(u64_t x)
{
	TH_saw_check_sanity++;
	checker_kernel_locked();
	BKL_UNLOCK();
	return x;
}

u64_t H2K_check_sched_mask(u64_t x)
{
	FAIL("Saw sched mask check");
	return x;
}

void test_thread(unsigned int arg)
{
	while (1) /* spin */;
}

unsigned long long int stack;

int main() 
{
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_thread_init();

	a.prio = b.prio = c.prio = 0;
	a.ugpgp = 0x12345;
	b.ugpgp = c.ugpgp = 0x0;
	b.status = c.status = H2K_STATUS_DEAD;
	a.trapmask = 0x55ffffff;
	a.vmblock = (H2K_vmblock_t *)0x1337beef;

	if (H2K_thread_create((u32_t)test_thread,((u32_t)(&stack)),0xdeadbeef,2,0x0,&a)
		!= 0xffffffff) FAIL("Created thread w/o storage");
	H2K_gp->free_threads = &b;
	b.next = &c;
	c.next = NULL;
	if (H2K_thread_create(((u32_t)test_thread)+1,((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= 0xffffffff) FAIL("Created thread w/ misaligned pc");
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack))+1,0xdeadbeef,2,0x0,&a) 
		!= 0xffffffff) FAIL("Created thread w/ misaligned sp");
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,902,0x0,&a) 
		!= 0xffffffff) FAIL("Created thread w/ misaligned sp");

	if (TH_saw_check_sanity != 0) FAIL("Called check_sanity on failure");

	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= (u32_t)(&b)) FAIL("Failed to create expected thread");
	if (TH_saw_check_sanity == 0) FAIL("Did not call check_sanity");
	if (H2K_gp->ready[2] != &b) FAIL("Thread inserted incorrectly into ready list");
	if (b.prio != 2) FAIL("thread priority set wrong");
	if (b.status == H2K_STATUS_DEAD) FAIL("status field incorrect");
	if (b.r0100 != 0x00000000deadbeefULL) FAIL("Incorrect argument");
	if (b.trapmask != 0x55ffffff) FAIL("Incorrect trap mask");
	if (b.continuation != H2K_interrupt_restore) FAIL("Incorrect continuation");
	if ((b.ssrelr & 0x00000000FFFFFFFF) != ((u32_t)test_thread)) FAIL("Incorrect return address");
	if ((b.ssrelr >> 32) != (a.ssrelr >> 32)) FAIL("Incorrect inheritance of SSR");
	if (b.ugpgp != a.ugpgp) FAIL("Incorrect inheritance of UGP/GP");
	if (b.vmblock != 0) FAIL("vmblock is non-NULL");

	TH_saw_check_sanity = 0;
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= (u32_t)(&c)) FAIL("Failed to create expected thread");
	if (TH_saw_check_sanity == 0) FAIL("Did not call check_sanity");

	if (H2K_thread_create((u32_t)test_thread,((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= 0xffffffff) FAIL("Created thread w/o storage");

	puts("TEST PASSED\n");
	return 0;
}

