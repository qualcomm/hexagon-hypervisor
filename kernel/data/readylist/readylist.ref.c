/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

void H2K_readylist_init(void) 
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->ready[i] = NULL;
	}
	H2K_gp->ready_valids = 0;
	H2K_gp->ready_validmask = 0xffffffff;
}

