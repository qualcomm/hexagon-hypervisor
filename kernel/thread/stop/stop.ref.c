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
#include <asid.h>
#include <stop.h>

void H2K_thread_stop(H2K_thread_context *me)
{       
        BKL_LOCK(&H2K_bkl);
        H2K_runlist_remove(me);
	H2K_asid_table_dec(me->ssr_asid);
        H2K_thread_context_clear(me);
	me->next = H2K_gp->free_threads;
	H2K_gp->free_threads = me;
	H2K_dosched(me,get_hwtnum());
}

