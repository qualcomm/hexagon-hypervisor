/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <runlist.h>
#include <readylist.h>
#include <dosched.h>
#include <hw.h>
#include <globals.h>
#include <yield.h>

/*
 * H2K_sched_yield
 * Run a different thread at the same prio, if available
 * TBD: detect nothing in readylist @ current prio and return
 */
void H2K_sched_yield(H2K_thread_context *me)
{
        BKL_LOCK(&H2K_bkl);
	if ((H2K_gp->ready_valids & (1<<me->prio)) == 0) {
		BKL_UNLOCK(&H2K_bkl);
		return;
	}
        H2K_runlist_remove(me);
        H2K_ready_append(me);
        H2K_dosched(me,me->hthread);
}

