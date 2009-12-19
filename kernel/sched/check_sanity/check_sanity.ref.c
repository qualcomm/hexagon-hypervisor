/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <check_sanity.h>
#include <c_std.h>
#include <context.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>

u64_t H2K_check_sanity(const u64_t retval)
{
	if (H2K_priomask == 0) {
		H2K_lowprio_notify();
	}
	if (H2K_runlist_worst_prio() IS_WORSE_THAN H2K_ready_best_prio()) {
		resched_int();
	} else if (H2K_wait_mask && (H2K_ready_valids /* & H2K_ready_validmask */)) {
		resched_int();
	}
#if 0
	/* EJP: ready valid mask not enabled... yet! */
	if ((H2K_runlist_valids & (~H2K_ready_validmask)) != 0) {
		H2K_lowprio_notify();
		H2K_resched_int();
	}
#endif

	return(retval);
}

u64_t H2K_check_sanity_unlock(const u64_t retval)
{
	call(H2K_check_sanity,retval);
	BKL_UNLOCK();
	return(retval);
}

