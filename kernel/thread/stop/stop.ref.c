/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

void BLASTK_thread_stop(BLASTK_thread_context *me)
{       
        BKL_LOCK(&BLASTK_bkl);
        runlist_remove(me);
        memset(me,0,sizeof(*me));
	me->next = BLASTK_free_threads;
	BLASTK_free_threads = me;
        return BLASTK_dosched_with_lock(me,get_hwtnum());
}

