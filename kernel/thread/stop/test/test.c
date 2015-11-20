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
#include <setjmp.h>

u32_t H2K_thread_id(H2K_thread_context *x);

jmp_buf env;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;

u32_t TH_saw_dosched = 0;
H2K_thread_context *TH_me = NULL;

void H2K_mem_alloc_release(u32_t *unused)
{
}

void H2K_mem_alloc_free(u32_t *ptr)
{
	return H2K_mem_alloc_release(ptr);
}

void H2K_mem_alloc_init();

u64_t *H2K_mem_alloc_get(u32_t size)
{
	return malloc(size);
}

void H2K_dosched(H2K_thread_context *me, u32_t hwtnum)
{
	if (me != TH_me) FAIL("Me passed incorrectly");
	if (hwtnum != get_hwtnum()) FAIL("hwtnum incorrect");
	TH_saw_dosched = 1;
	checker_kernel_locked();
	BKL_UNLOCK();
	longjmp(env,1);
}

void TH_thread_stop(H2K_thread_context *me)
{
	if (setjmp(env) == 0) {
		H2K_thread_stop(0x01234567, me);
	}
}

/* FIXME: check for interrupt post to parent CPU */
int main() 
{
	H2K_vmblock_t myblock;
	H2K_vmblock_t *vmblock = &myblock;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_thread_init();

	a.prio = 2;
	b.prio = 2;
	c.prio = 2;
	a.hthread = 0;
	b.hthread = 1;
	c.hthread = 2;
	a.vmblock = vmblock;
	b.vmblock = vmblock;
	c.vmblock = vmblock;
	//puts("C");
	vmblock->free_threads = NULL;
	if (vmblock->free_threads != NULL) FAIL("free threads not clear");
	H2K_runlist_push(&a);
	if (H2K_gp->runlist[a.hthread] != &a) FAIL("Thread not in expected place in runlist (0)");
	H2K_runlist_push(&b);
	if (H2K_gp->runlist[b.hthread] != &b) FAIL("Thread not in expected place in runlist (1)");
	TH_me = &a;
	a.prev = &a;
	TH_saw_dosched = 0;
	//puts("D");
	TH_thread_stop(TH_me);
	//puts("E");

	if (TH_saw_dosched == 0) FAIL("Dosched not called");
	//puts("f");
	if (a.prev != 0) FAIL("thread not cleared");
	//puts("g");
	if (vmblock->free_threads != &a) FAIL("free thread list incorrect");
	//puts("h");
	if (a.next != 0) FAIL("Free thread list incorrect");
	//puts("i");
	if (H2K_gp->runlist[a.hthread] == &a) FAIL("Thread not removed from runlist");
	//puts("B");
	TH_saw_dosched = 0;
	TH_me = &b;
	b.prev = &b;
	TH_thread_stop(TH_me);
	if (TH_saw_dosched == 0) FAIL("Dosched not called");
	if (b.prev != 0) FAIL("thread not cleared");
	if (vmblock->free_threads != &b) FAIL("free thread list incorrect");
	if (b.next != &a) FAIL("Free thread list incorrect");
	if (H2K_gp->runlist[b.hthread] == &b) FAIL("Thread not removed from runlist");
	if (H2K_gp->runlist[a.hthread] != NULL) FAIL("Unexpected runlist");
	if (H2K_gp->runlist[b.hthread] != NULL) FAIL("Unexpected runlist");
	puts("A");
	H2K_runlist_init();
	H2K_lowprio_init();
	H2K_readylist_init();
	H2K_thread_init();
	a.prio = 2;
	b.prio = 2;
	c.prio = 2;
	vmblock->free_threads = NULL;
	if (vmblock->free_threads != NULL) FAIL("free threads not clear");
	H2K_runlist_push(&a);
	if (H2K_gp->runlist[a.hthread] != &a) FAIL("Thread not in expected place in runlist (2)");
	H2K_runlist_push(&b);
	if (H2K_gp->runlist[b.hthread] != &b) FAIL("Thread not in expected place in runlist (3)");
	H2K_runlist_push(&c);
	if (H2K_gp->runlist[c.hthread] != &c) FAIL("Thread not in expected place in runlist (4)");
	TH_me = &a;
	a.prev = &a;
	TH_saw_dosched = 0;
	TH_thread_stop(TH_me);

	if (TH_saw_dosched == 0) FAIL("Dosched not called");
	if (a.prev != 0) FAIL("thread not cleared");
	if (vmblock->free_threads != &a) FAIL("free thread list incorrect");
	if (a.next != 0) FAIL("Free thread list incorrect");
	if (H2K_gp->runlist[a.hthread] == &a) FAIL("Thread not removed from runlist");

	puts("TEST PASSED\n");
	return 0;
}

