/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <readylist.h>
#include <checker_help.h>
#include <checker_ready.h>
#include <checker_ring.h>
#include <globals.h>

s32_t checker_ready()
{
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		if (H2K_ready_prio_valid(i)) {
			if (NULL == H2K_gp->ready[i]) FAIL("Valid bit set but readylist null");
			checker_ring((H2K_ringnode_t *)H2K_gp->ready[i]);
		} else {
			if (H2K_gp->ready[i]) FAIL("valid bit clear but readylist non-null");
		}
	}
	return 1;
}

