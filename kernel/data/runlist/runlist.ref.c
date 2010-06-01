/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <runlist.h>

void H2K_runlist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->runlist[i] = NULL;
	}
	H2K_gp->runlist_valids = 0;
}
