/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

unsigned long long int BLASTK_check_sanity(const unsigned long long int retval)
{
	if (BLASTK_priomask == 0) {
		lowprio_notify();
	}
	if (get_worst_running_prio() IS_WORSE_THAN get_best_ready_prio()) {
		resched_int();
	}
	else if (BLASTK_wait_mask && (BLASTK_ready_valids & BLASTK_ready_validmask)) {
		resched_int();
	}
	if ((BLASTK_runlist_valids & (~BLASTK_ready_validmask)) != 0) {
		lowprio_notify();
		resched_int();
	}
	return retval;
}

unsigned long long int BLASTK_check_sanity_unlock(const unsigned long long int retval)
{
	BLASTK_check_sanity(retval);
	BKL_unlock();
	return retval;
}

