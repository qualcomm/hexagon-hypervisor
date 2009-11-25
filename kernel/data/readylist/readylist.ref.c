/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

BLASTK_thread_context *BLASTK_ready[MAX_PRIOS] IN_SECTION(".data.sched.ready");
u32_t BLASTK_ready_valids IN_SECTION(".data.sched.ready");

void BLASTK_readylist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		BLASTK_ready[i] = NULL;
	}
	BLASTK_ready_valids = 0;
}

