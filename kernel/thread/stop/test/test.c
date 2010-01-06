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

static H2K_thread_context a,b;

u32_t TH_saw_dosched = 0;
H2K_thread_context *TH_me = NULL;

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
		H2K_thread_stop(me);
	}
}

int main() 
{
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_thread_init();

	a.prio = 2;
	b.prio = 2;
	if (H2K_free_threads != NULL) FAIL("free threads not clear");
	H2K_runlist_push(&a);
	if (H2K_runlist[2] != &a) FAIL("Thread not in expected place in runlist");
	H2K_runlist_push(&b);
	if (H2K_runlist[2] != &b) FAIL("Thread not in expected place in runlist");
	TH_me = &a;
	a.prev = &a;
	TH_saw_dosched = 0;
	TH_thread_stop(TH_me);

	if (TH_saw_dosched == 0) FAIL("Dosched not called");
	if (a.prev != 0) FAIL("thread not cleared");
	if (H2K_free_threads != &a) FAIL("free thread list incorrect");
	if (a.next != 0) FAIL("Free thread list incorrect");
	if (H2K_runlist[2] == &a) FAIL("Thread not removed from runlist");
	if (H2K_runlist[2]->next == &a) FAIL("Thread not removed from runlist");

	TH_saw_dosched = 0;
	TH_me = &b;
	b.prev = &b;
	TH_thread_stop(TH_me);
	if (TH_saw_dosched == 0) FAIL("Dosched not called");
	if (b.prev != 0) FAIL("thread not cleared");
	if (H2K_free_threads != &b) FAIL("free thread list incorrect");
	if (b.next != &a) FAIL("Free thread list incorrect");
	if (H2K_runlist[2] == &b) FAIL("Thread not removed from runlist");
	if (H2K_runlist[2] != NULL) FAIL("Unexpected runlist");

	puts("TEST PASSED\n");
	return 0;
}

