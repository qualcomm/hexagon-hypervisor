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
 */
void H2K_sched_yield(H2K_thread_context *me)
{
        BKL_LOCK(&H2K_bkl);
	if (!H2K_ready_prio_valid(me->prio)) {
		BKL_UNLOCK(&H2K_bkl);
		return;
	}
        H2K_runlist_remove(me);
        H2K_ready_append(me);
        H2K_dosched(me,me->hthread);
}

