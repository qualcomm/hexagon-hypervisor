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
#include <resched.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_kernel_locked.h>
#include <checker_runlist.h>
#include <checker_ready.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context *TB_in;

u32_t TB_saw_dosched = 0;

void H2K_dosched(H2K_thread_context *in, int hthread)
{
	if (in != TB_in) FAIL("Unexpected thread passed to dosched");
	if (hthread != 0) FAIL("Unexpected hardware thread");
	if (H2K_wait_mask != 0) FAIL("Set bit in wait_mask");
	TB_saw_dosched ++;
	checker_kernel_locked();
	BKL_UNLOCK();
	checker_runlist();
	checker_ready();
}

static H2K_thread_context a,b,c;

int main() 
{
	H2K_readylist_init();
	H2K_runlist_init();
	H2K_lowprio_init();
	a.prio = b.prio = c.prio = 2;
	TB_in = NULL;
	H2K_resched(0,TB_in,0);
	if (TB_saw_dosched == 0) FAIL("did not do a resched");
	TB_saw_dosched = 0;
	H2K_wait_mask = 1;
	H2K_resched(0,TB_in,0);
	if (TB_saw_dosched == 0) FAIL("Did not do a resched");
	H2K_wait_mask = 0;
	TB_saw_dosched = 0;
	TB_in = &a;
	H2K_runlist_push(&a);
	H2K_runlist_push(&c);
	H2K_resched(0,TB_in,0);
	if (TB_saw_dosched == 0) FAIL("Did not do a resched");
	if (H2K_runlist[2] != &c) FAIL("Unexpected thread in runlist");
	if (H2K_ready[2] != &a) FAIL("Unexpected thread in readylist");
	H2K_wait_mask = 0;
	TB_saw_dosched = 0;
	TB_in = &c;
	H2K_resched(0,TB_in,0);
	if (TB_saw_dosched == 0) FAIL("Did not do a resched");
	if (H2K_ready[2] != &a) FAIL("Unexpected thread in readylist");
	if (H2K_ready[2]->next != &c) FAIL("Unexpected thread in readylist");
	puts("PASS");
	return 0;
}

