/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <runlist.h>
#include <checker_help.h>
#include <checker_runlist.h>
#include <checker_ring.h>

s32_t checker_runlist()
{
	u32_t i;
	for (i = 0; i < H2K_gp->hthreads; i++) {
		if (0 <= H2K_gp->runlist_prios[i] && H2K_gp->runlist_prios[i] <= MAX_PRIO) {
			if (H2K_gp->runlist_prios[i] != H2K_gp->runlist[i]->prio) FAIL("runlist_prios does not match priority of scheduled thread");
		} else {
			if (H2K_gp->runlist[i]) FAIL("Priority out of range but readylist non-null");
		}
	}
	return 1;
}

