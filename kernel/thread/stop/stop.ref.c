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
#include <cpuint.h>
#include <id.h>
#include <alloc.h>

void H2K_thread_stop(s32_t status, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;
	H2K_thread_context *parent_context;
	H2K_vmblock_t *parent_vmblock;

	BKL_LOCK(&H2K_bkl);

	vmblock->num_cpus--;
	vmblock->status = status;

	if (vmblock->num_cpus == 0) { // all stopped, signal parent
		parent_context = H2K_id_to_context(vmblock->parent);
		if (parent_context != NULL
				&& parent_context->status != H2K_STATUS_DEAD) { // parent exists
			parent_vmblock = parent_context->vmblock;

			H2K_vm_cpuint_post_locked(parent_vmblock, parent_context, H2K_VM_CHILDINT, parent_vmblock->intinfo);
		} else { // no parent; just deallocate.
			/* Can't free immediately because H2K_switch reads from *me */
			H2K_mem_alloc_release((u32_t *)vmblock);  
		}
	}

	H2K_timer_cancel_withlock(me);
	H2K_runlist_remove(me);
	H2K_asid_table_dec(me->ssr_asid);
	H2K_thread_context_clear(me);
	me->next = vmblock->free_threads;
	vmblock->free_threads = me;
	H2K_dosched(me,get_hwtnum());
}

