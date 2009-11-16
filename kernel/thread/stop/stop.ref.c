/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <thread.h>
#include <dosched.h>
#include <runlist.h>

void BLASTK_thread_stop(BLASTK_thread_context *me)
{       
        BKL_LOCK(&BLASTK_bkl);
        BLASTK_runlist_remove(me);
        BLASTK_thread_context_clear(me);
	me->next = BLASTK_free_threads;
	BLASTK_free_threads = me;
        return BLASTK_dosched(me,get_hwtnum());
}

