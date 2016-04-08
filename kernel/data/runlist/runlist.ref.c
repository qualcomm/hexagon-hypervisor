/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <runlist.h>

void H2K_runlist_init(void) 
{
	u32_t i;
	for (i = 0; i < sizeof(H2K_gp->runlist_prios)/sizeof(H2K_gp->runlist_prios[0]); i++) {
		if (i < H2K_gp->hthreads) { /* EJP: FIXME: why is this needed? */
			H2K_gp->runlist[i] = NULL;
		}
		H2K_gp->runlist_prios[i] = -1;
	}
}
