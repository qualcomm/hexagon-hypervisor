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
    u32_t best = H2K_ready_best_prio();
    H2K_set_bestwait(best); // This is blind for waiting thread- with prio -1, ie 255, and ready thread with prio 255- This won't fire an interrupt

    if (H2K_get_bestwait() == BESTWAIT_MASK) { //bestwait fired
        return(retval);
    }

    if (H2K_gp->wait_mask && best < MAX_PRIOS) { //We reach here only if ready thread's prio is 255. If it was 254 bestwait see waiting's prio 255 > 254 and fires.
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
