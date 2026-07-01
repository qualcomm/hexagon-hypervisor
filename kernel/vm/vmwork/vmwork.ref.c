/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmint.h>
#include <vmwork.h>
#include <vmevent.h>
#include <stop.h>
#include <context.h>
#include <atomic.h>
#include <sample.h>
#include <h2_common_defs.h>

s32_t H2K_vm_do_work_withlock(H2K_thread_context *me)
{
	if (me->vmstatus & H2K_VMSTATUS_KILL) {
		H2K_thread_stop_withlock(me->vmblock->status, me);
	}

	if (!(me->vmstatus & H2K_VMSTATUS_VMWORK)) return -1;

	H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	return H2K_vm_check_interrupts(me);
}
