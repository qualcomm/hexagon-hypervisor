/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLASTK_sched_yield
 * Run a different thread at the same prio, if available
 */
void BLASTK_sched_yield(BLASTK_thread_context *me)
{       
        BKL_LOCK(&BLASTK_bkl);
        runlist_remove(me);
        ready_append(me);
        BLASTK_dosched_with_lock(me,me->hthread);
}

