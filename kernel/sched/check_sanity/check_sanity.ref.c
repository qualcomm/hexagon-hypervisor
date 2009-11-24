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

u64_t BLASTK_check_sanity(const u64_t retval)
{
	if (BLASTK_priomask == 0) {
		BLASTK_lowprio_notify();
	}
	if (BLASTK_runlist_worst_prio() IS_WORSE_THAN BLASTK_ready_best_prio()) {
		resched_int();
	}
	else if (BLASTK_wait_mask && (BLASTK_ready_valids /* & BLASTK_ready_validmask */)) {
		resched_int();
	}
#if 0
	/* EJP: ready valid mask not enabled... yet! */
	if ((BLASTK_runlist_valids & (~BLASTK_ready_validmask)) != 0) {
		BLASTK_lowprio_notify();
		BLASTK_resched_int();
	}
#endif

	return(retval);
}

u64_t BLASTK_check_sanity_unlock(const u64_t retval)
{
	BLASTK_check_sanity(retval);
	BKL_UNLOCK();
	return(retval);
}

