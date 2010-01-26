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

static inline void checker_runlist_check_ll(H2K_ringnode_t *x)
{
	while (x) x = x->next;
}

s32_t checker_runlist()
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		if (H2K_gp->runlist_valids & (1<<i)) {
			if (NULL == H2K_gp->runlist[i]) FAIL("Valid bit clear but readylist null");
			checker_runlist_check_ll((H2K_ringnode_t *)H2K_gp->runlist[i]);
		} else {
			if (H2K_gp->runlist[i]) FAIL("valid bit clear but readylist non-null");
		}
	}
	return 1;
}

