/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <runlist.h>
#include <hw.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;

u32_t H2K_runlist_worst_prio_TB()
{
	return H2K_runlist_worst_prio();
}

void H2K_runlist_push_TB(H2K_thread_context *thread)
{
	H2K_runlist_push(thread);
}

void H2K_runlist_remove_TB(H2K_thread_context *thread)
{
	H2K_runlist_remove(thread);
}

int main() 
{
	H2K_runlist_valids = 0xdeadbeef;
	H2K_runlist[0] = &a;
	H2K_runlist_init();
	if (H2K_runlist_valids != 0) FAIL("runlist_init failed to set valids");
	if (H2K_runlist[0] != 0) FAIL("runlist_init failed to clear array");
	if (H2K_runlist_worst_prio_TB() <= MAX_PRIO) FAIL("cleared runlist worst prio <= MAX_PRIO");

	c.prio = b.prio = a.prio = 2;
	H2K_runlist_push_TB(&a);
	if (H2K_runlist[2] != &a) FAIL("runlist_append failed (0) ");
	if ((H2K_runlist_valids & (1<<2)) == 0) FAIL("runlist_append failed (1) ");
	if (H2K_runlist[2]->next != NULL) FAIL("runlist_append failed (2) ");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("runlist worst prio (4) ");

	H2K_runlist_push_TB(&b);
	if (H2K_runlist[2] != &b) FAIL("runlist_append failed (5)");
	if ((H2K_runlist_valids & (1<<2)) == 0) FAIL("runlist_append failed (6)");
	if (H2K_runlist[2]->next != &a) FAIL("runlist_append failed (7)");
	if (H2K_runlist[2]->next->next != NULL) FAIL("runlist_append failed (9)");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("runlist worst prio");
	
	H2K_runlist_push_TB(&c);

	H2K_runlist_remove_TB(&a);
	H2K_runlist_remove_TB(&c);
	if (H2K_runlist[2] != &b) FAIL("ready_remove failed");
	if ((H2K_runlist_valids & (1<<2)) == 0) FAIL("ready_remove failed");
	if (H2K_runlist[2]->next != NULL) FAIL("ready_remove failed");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("Ready best prio");

	H2K_runlist_remove_TB(&b);
	if (H2K_runlist[2] != NULL) FAIL("ready_remove failed");
	if (H2K_runlist_valids != 0) FAIL("ready_remove failed");
	if (H2K_runlist_worst_prio_TB() <= MAX_PRIO) FAIL("Ready best prio");

	puts("TEST PASSED\n");
	return 0;
}

