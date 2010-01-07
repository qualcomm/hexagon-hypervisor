/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <runlist.h>
#include <lowprio.h>
#include <context.h>
#include <hw.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

void H2K_sched_yield(H2K_thread_context *me);

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context *TB_in;

u32_t TB_saw_dosched = 0;

jmp_buf env;

void H2K_dosched(H2K_thread_context *in, int hthread)
{
	if (in != TB_in) FAIL("Unexpected thread passed to dosched");
	TB_saw_dosched ++;
	longjmp(env,1);
}

static H2K_thread_context a,b,c,d;

void TH_sched_yield(H2K_thread_context *me)
{
	if (setjmp(env) == 0) {
		H2K_sched_yield(me);
	}
}

int main() 
{
	H2K_readylist_init();
	H2K_runlist_init();
	H2K_lowprio_init();
	a.prio = b.prio = c.prio = d.prio = 2;
	TB_in = &a;
	H2K_runlist_push(&a);
	H2K_runlist_push(&c);
	TH_sched_yield(TB_in);
	if (TB_saw_dosched != 0) FAIL("Did a resched");
	H2K_ready_append(&b);
	TH_sched_yield(TB_in);
	if (TB_saw_dosched == 0) FAIL("Did not do a resched");
	BKL_UNLOCK();
	TB_in = &c;
	TB_saw_dosched = 0;
	TH_sched_yield(TB_in);
	if (TB_saw_dosched == 0) FAIL("Did not do a resched");
	BKL_UNLOCK();
	H2K_readylist_init();
	H2K_runlist_init();
	H2K_lowprio_init();
	H2K_runlist_push(&a);
	H2K_runlist_push(&c);
	H2K_runlist_push(&d);
	H2K_ready_append(&b);
	TB_in = &a;
	TH_sched_yield(TB_in);
	if (TB_saw_dosched == 0) FAIL("DId not do a resched");
	BKL_UNLOCK();
	puts("TEST PASSED\n");
	return 0;
}

