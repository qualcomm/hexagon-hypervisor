/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <lowprio.h>
#include <runlist.h>
#include <readylist.h>
#include <hw.h>
#include <stdio.h>

void FAIL(const char *x)
{
	puts("FAIL");
	puts(x);
	exit(1);
}

void h2_init();

H2K_thread_context a,b;

void H2K_lowprio_notify_TB()
{
	return H2K_lowprio_notify();
}

void H2K_raise_lowprio_TB()
{
	return H2K_raise_lowprio();
}

typedef void (*TB_func)();

TB_func notify = H2K_lowprio_notify_TB;
TB_func raise = H2K_raise_lowprio_TB;

int main()
{
	int i;
	for (i = 0; i < 1000; i++) {
		h2_init();
	}
	a.prio = b.prio = 2;
	a.hthread = b.hthread = 1;
	if ((get_imask(1) & 1) != 0) FAIL("T1 should be idle at boot");
	H2K_runlist_push(&a);
	raise();
	if ((get_imask(1) & 1) != 0) FAIL("should not have raised if idle");
	H2K_wait_mask = 0;
	H2K_priomask = 0x2;
	raise();
	if ((get_imask(1) & 1) == 0) FAIL("should have raised T1");
	H2K_priomask = 0;
	notify();
	if ((get_imask(1) & 1) != 0) FAIL("should have notified T1");
	puts("PASS");
	return 0;
}

