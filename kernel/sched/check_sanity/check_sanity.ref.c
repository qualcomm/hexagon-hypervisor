/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <check_sanity.h>
#include <c_std.h>
#include <max.h>
#include <context.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <globals.h>
#include <hw.h>

u64_t H2K_check_sanity(const u64_t retval)
{
	if (H2K_gp->priomask == 0) {
		H2K_lowprio_notify();
	}
	u32_t best = H2K_ready_best_prio();
	H2K_set_bestwait(best);
	if (H2K_get_bestwait() != BESTWAIT_MASK && H2K_gp->wait_mask && best < MAX_PRIOS) {
		resched_int();
	}
	return(retval);
}

u64_t H2K_check_sanity_unlock(const u64_t retval)
{
	call(H2K_check_sanity,retval);
	BKL_UNLOCK();
	return(retval);
}
