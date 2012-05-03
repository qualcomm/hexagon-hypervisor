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
#include <timer.h>
#include <vm.h>

void H2K_thread_stop(H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;

	BKL_LOCK(&H2K_bkl);

	if (vmblock) {  // is a vcpu, EJP: will always be true soon
		vmblock->num_cpus--;
	}
	H2K_timer_cancel_withlock(me);
	H2K_runlist_remove(me);
	H2K_asid_table_dec(me->ssr_asid);
	H2K_thread_context_clear(me);
	me->next = vmblock->free_threads;
	vmblock->free_threads = me;
	H2K_dosched(me,get_hwtnum());
}

