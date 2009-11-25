/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <runlist.h>

BLASTK_thread_context *BLASTK_runlist[MAX_PRIOS] IN_SECTION(".data.sched.runlist");
u32_t BLASTK_runlist_valids IN_SECTION(".data.sched.runlist");

void BLASTK_runlist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		BLASTK_runlist[i] = NULL;
	}
	BLASTK_runlist_valids = 0;
}
