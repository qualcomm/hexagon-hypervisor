/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <runlist.h>
#include <readylist.h>
#include <dosched.h>
#include <hw.h>

/*
 * BLASTK_sched_yield
 * Run a different thread at the same prio, if available
 * TBD: detect nothing in readylist @ current prio and return
 */
void BLASTK_sched_yield(BLASTK_thread_context *me)
{
        BKL_LOCK(&BLASTK_bkl);
	if ((BLASTK_ready_valids & (1<<me->prio)) == 0) {
		BKL_UNLOCK(&BLASTK_bkl);
		return;
	}
        BLASTK_runlist_remove(me);
        BLASTK_ready_append(me);
        BLASTK_dosched(me,me->hthread);
}

