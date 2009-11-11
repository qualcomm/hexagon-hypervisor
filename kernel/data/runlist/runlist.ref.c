/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <runlist.h>

BLASTK_thread_context *BLASTK_runlist[MAX_PRIOS];
unsigned int BLASTK_runlist_valids;

void BLASTK_runlist_init(void) 
{
	int i;
	for (i = 0; i < MAX_PRIOS; i++) {
		BLASTK_runlist[i] = NULL;
	}
	BLASTK_runlist_valids = 0;
}

