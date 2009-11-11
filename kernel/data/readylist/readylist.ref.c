/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

BLASTK_thread_context *BLASTK_ready[MAX_PRIOS];
unsigned int BLASTK_ready_valids;

void BLASTK_readylist_init(void) 
{
	int i;
	for (i = 0; i < MAX_PRIOS; i++) {
		BLASTK_ready[i] = NULL;
	}
	BLASTK_ready_valids = 0;
}

