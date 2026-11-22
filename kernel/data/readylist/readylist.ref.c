/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

#if CLUSTER_SCHED
H2K_thread_context *H2K_ready_getbest_asm(u32_t hthread)
{
	return H2K_ready_getbest(hthread);
}
#endif

void H2K_readylist_init(void)
{
#ifdef TESTING
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->ready[i] = NULL;
	}
	for (i = 0; i < MAX_PRIOS/64; i++) {
		H2K_gp->ready_valids[i] = 0;
	}
#endif
}
