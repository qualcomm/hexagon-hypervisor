/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

H2K_thread_context *H2K_ready[MAX_PRIOS] IN_SECTION(".data.sched.ready");
u32_t H2K_ready_valids IN_SECTION(".data.sched.ready");
u32_t H2K_ready_validmask IN_SECTION(".data.sched.ready");

void H2K_readylist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_ready[i] = NULL;
	}
	H2K_ready_valids = 0;
	H2K_ready_validmask = 0;
}

