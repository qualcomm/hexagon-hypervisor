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
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;
static u32_t hthread_mask = 0;

/* Create real versions of inlined functions */

s32_t H2K_runlist_worst_prio_TB()
{
	return H2K_runlist_worst_prio();
}

s32_t H2K_runlist_worst_prio_hthread_TB()
{
	return H2K_runlist_worst_prio_hthread();
}

u32_t H2K_runlist_push_TB(H2K_thread_context *thread)
{
	H2K_runlist_push(thread);
	hthread_mask |= (0x1 << thread->hthread);
	return hthread_mask;
}

u32_t H2K_runlist_remove_TB(H2K_thread_context *thread)
{
	H2K_runlist_remove(thread);
	hthread_mask &= ~(0x1 << thread->hthread);
	return hthread_mask;
}

u32_t H2K_runlist_prio_hthreads_TB(u32_t hthreads, u32_t prio)
{
	return H2K_runlist_prio_hthreads(hthreads, prio);
}

u32_t H2K_runlist_worst_prio_hthread_OK()
{
	s32_t hthread = H2K_runlist_worst_prio_hthread_TB();
	s32_t prio = H2K_runlist_worst_prio_TB();
	if (prio < 0 || prio > MAX_PRIO) {
		return hthread < 0 || hthread > H2K_gp->hthreads;
	}
	return H2K_gp->runlist[hthread] && H2K_gp->runlist[hthread]->prio == prio;
}

int main() 
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	hthread_mask = 0;
	H2K_gp->hthreads = 0;
	if (H2K_runlist_worst_prio_TB() <= MAX_PRIO) FAIL("hthreads-off runlist worst prio in [0,MAX_PRIO]");
	if (H2K_runlist_prio_hthreads_TB(hthread_mask,0) != hthread_mask) FAIL("runlist_prio_hthreads failed default");
	H2K_gp->runlist[0] = &a;
	H2K_gp->hthreads = get_hthreads();
	H2K_runlist_init();
	if (H2K_gp->runlist[0] != 0) FAIL("runlist_init failed to clear array");
	if (0 <= H2K_gp->runlist_prios[1] && H2K_gp->runlist_prios[1] <= MAX_PRIO) FAIL("runlist_init failed to clear prios");
	if (H2K_runlist_worst_prio_TB() <= MAX_PRIO) FAIL("cleared runlist worst prio in [0,MAX_PRIO]");
	if (!H2K_runlist_worst_prio_hthread_OK()) FAIL("cleared runlist worst prio hthread in [0,H2K_gp->hthreads)");

	b.prio = a.prio = 2;
	c.prio = 3;
	a.hthread = 0;
	b.hthread = 1;
	c.hthread = 2;

	u32_t hthreads_c = H2K_runlist_push_TB(&c);
	u32_t hthreads_zero = H2K_runlist_remove_TB(&c);
	if (hthreads_zero != 0) FAIL("runlist_push_remove combo failed");

	u32_t hthreads_a = H2K_runlist_push_TB(&a);
	if (H2K_gp->runlist[0] != &a) FAIL("runlist_push failed (0)");
	if (H2K_gp->runlist_prios[0] != 2) FAIL("runlist_push failed (1)");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("runlist_worst_prio failed (3)");
	if (!H2K_runlist_worst_prio_hthread_OK()) FAIL("runlist_worst_prio_hthread failed (30)");
	if (H2K_runlist_prio_hthreads_TB(hthreads_a,a.prio) != hthreads_a) FAIL("runlist_prio_hthreads failed (40)");

	u32_t hthreads_ab = H2K_runlist_push_TB(&b);
	if (H2K_gp->runlist[1] != &b) FAIL("runlist_push failed (4)");
	if (H2K_gp->runlist_prios[1] != 2) FAIL("runlist_push failed (5)");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("runlist_worst_prio failed (7)");
	if (!H2K_runlist_worst_prio_hthread_OK()) FAIL("runlist_worst_prio_hthread failed (31)");
	if (H2K_runlist_prio_hthreads_TB(hthreads_ab,b.prio) != hthreads_ab) FAIL("runlist_prio_hthreads failed (41)");

	u32_t hthreads_abc = H2K_runlist_push_TB(&c);
	if (H2K_runlist_worst_prio_TB() != 3) FAIL("runlist_worst_prio (8)");
	if (!H2K_runlist_worst_prio_hthread_OK()) FAIL("runlist_worst_prio_hthread failed (32)");
	if (H2K_runlist_prio_hthreads_TB(hthreads_abc,c.prio) != hthreads_c) FAIL("runlist_prio_hthreads failed (42)");

	u32_t hthreads_bc = H2K_runlist_remove_TB(&a);
	u32_t hthreads_b = H2K_runlist_remove_TB(&c);
	if (H2K_gp->runlist[0] != NULL) FAIL("runlist_remove failed (9)");
	if (H2K_gp->runlist[1] != &b) FAIL("runlist_remove failed (10)");
	if (H2K_gp->runlist[2] != NULL) FAIL("runlist_remove failed (11)");
	if (H2K_gp->runlist_prios[1] != 2) FAIL("runlist_remove failed (12)");
	if (0 <= H2K_gp->runlist_prios[2] && H2K_gp->runlist_prios[2] <= MAX_PRIO) FAIL("runlist_remove failed (13)");
	if (H2K_runlist_worst_prio_TB() != 2) FAIL("runlist_worst_prio (17)");
	if (!H2K_runlist_worst_prio_hthread_OK()) FAIL("runlist_worst_prio_hthread failed (33)");
	if (H2K_runlist_prio_hthreads_TB(hthreads_bc,b.prio) != hthreads_bc) FAIL("runlist_prio_hthreads failed (43)");
	if (H2K_runlist_prio_hthreads_TB(hthreads_b,b.prio) != hthreads_b) FAIL("runlist_prio_hthreads failed (44)");

	u32_t hthreads_none = H2K_runlist_remove_TB(&b);
	if (H2K_gp->runlist[1] != NULL) FAIL("runlist_remove failed (18)");
	if (0 <= H2K_gp->runlist_prios[1] && H2K_gp->runlist_prios[1] <= MAX_PRIO) FAIL("runlist_remove failed (19)");
	if (H2K_runlist_worst_prio_TB() <= MAX_PRIO) FAIL("runlist_worst_prio (21)");
	if (hthreads_none != hthreads_zero) FAIL("runlist_hthreads none-zero comparison failed (45)");

	puts("TEST PASSED\n");
	return 0;
}

