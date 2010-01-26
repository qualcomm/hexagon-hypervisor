/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <runlist.h>

//H2K_thread_context *H2K_runlist[MAX_PRIOS] IN_SECTION(".data.sched.runlist");
//u32_t H2K_runlist_valids IN_SECTION(".data.sched.runlist");

void H2K_runlist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->runlist[i] = NULL;
	}
	H2K_gp->runlist_valids = 0;
}
