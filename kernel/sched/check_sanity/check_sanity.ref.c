/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  Sample optional-automatic-assertion-checking-at-return-time  */
#define RETURN(retval) {\
	OUTPUT_ASSERTION;\
	return(retval);\
}

u64_t BLASTK_check_sanity(const u64_t retval)
{
	u64_t arg0 = retval;  /*  used for assertion  */

/*  Possibly filled in via Sphinx extension  */
#ifdef CHECK_ASSERTIONS
assert(kernel_locked());
#define OUTPUT_ASSERTION assert(retval == arg0);
#endif

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

	RETURN(retval);
}

u64_t BLASTK_check_sanity_unlock(const u64_t retval)
{
	u64_t arg0 = retval;  /*  used for assertion  */

/*  Possibly filled in via Sphinx extension  */
#ifdef CHECK_ASSERTIONS
assert(kernel_locked());
#define OUTPUT_ASSERTION assert(retval == arg0);\
assert(kernel_unlocked());
#endif

	BLASTK_check_sanity(retval);
	BKL_unlock();
	RETURN(retval);
}

