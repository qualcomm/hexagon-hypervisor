/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>
#include <globals.h>
#include <idtype.h>
#include <id.h>
#include <alloc.h>
#include <atomic.h>
#include <futex.h>
#include <popup.h>
#include <readylist.h>
#include <vmdefs.h>
#include <vmipi.h>
#include <vmop.h>
#include <hw.h>

typedef s32_t (*vmop_ptr_t)(vmop_t, u32_t, u32_t, u32_t, u32_t, u32_t, H2K_thread_context *);

static const vmop_ptr_t H2K_vmoptab[VMOP_MAX] IN_SECTION(".data.config.config") = {
	H2K_vmop_boot,
	H2K_vmop_status,
	H2K_vmop_free,
	H2K_vmop_kill_thread,
	H2K_vmop_kill_vm
};

s32_t H2K_vmop(vmop_t op, u32_t val1, u32_t val2, u32_t val3, u32_t val4, u32_t val5,  H2K_thread_context *me)
{
	if (op >= VMOP_MAX) return -1;
	return H2K_vmoptab[op](0, val1, val2, val3, val4, val5, me);
}

s32_t H2K_vmop_boot(vmop_t unused0, u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me) {

	H2K_vmblock_t *vmblock;

	/* FIXME: ? Lock to prevent concurrent boots of one vm? */

	/* Can only boot a child vm (that isn't running) */
	if (vm < H2K_ID_MAX_VMS && H2K_gp->vmblocks[vm] != NULL) { // ok
		vmblock = H2K_gp->vmblocks[vm];
		if ((me != NULL && me->id.vmidx != vmblock->parent.vmidx)
				|| vmblock->num_cpus > 0) return -1;
	} else {
		return -1;
	}

	return H2K_thread_create_no_squash(pc, sp, arg1, prio, H2K_gp->vmblocks[vm], me);
}

s32_t H2K_vmop_status(vmop_t unused0, u32_t op, u32_t vm, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) {

	H2K_vmblock_t *vmblock;
	s32_t ret = -1;

	if (NULL == me) return ret;
	if (vm >= H2K_ID_MAX_VMS) return ret;

	if (VMOP_STATUS_VMIDX_SELF == vm) {  // self
		vmblock = me->vmblock;
	} else {
		if (NULL == H2K_gp->vmblocks[vm]) return ret;
		vmblock = H2K_gp->vmblocks[vm];
	}
	/* Can only query child vm or self */
	if ( me->id.vmidx != vmblock->vmidx
			 && me->id.vmidx != vmblock->parent.vmidx) return ret;

	BKL_LOCK();
	switch ((vmop_status_t)op) {
	case VMOP_STATUS_STATUS:
		ret = vmblock->status;
		break;
	case VMOP_STATUS_CPUS:
		ret = vmblock->num_cpus;
		break;
	default:
		break;
	}
	BKL_UNLOCK();
	return ret;
}

s32_t H2K_vmop_free(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) {

	H2K_vmblock_t *vmblock;

	/* Can only free a child vm (that isn't running) */
	if (vm < H2K_ID_MAX_VMS && H2K_gp->vmblocks[vm] != NULL) { // ok
		vmblock = H2K_gp->vmblocks[vm];
		if ((me != NULL && me->id.vmidx != vmblock->parent.vmidx)
				|| vmblock->num_cpus > 0) return -1;
	} else {
		return -1;
	}

	H2K_mem_alloc_free((u32_t *)vmblock);
	H2K_gp->vmblocks[vm] = NULL;

	return 0;
}

/* FIXME: This duplicates the state-switch in H2K_vm_int_deliver_locked because
 * kill must ignore guest IE (the target may have disabled interrupts) and must
 * not overwrite target->r00 with an interrupt number.  Future work: factor the
 * "thread cancellation" out of int delivery so both call a shared primitive
 * (would also need updating vmint.v4opt.S). */
static void kill_thread_locked(H2K_vmblock_t *vmblock, H2K_thread_context *target)
{
	if (target->status == H2K_STATUS_DEAD) return;
	H2K_atomic_or(&target->atomic_status_word, H2K_VMSTATUS_KILL | H2K_VMSTATUS_VMWORK);
	switch (target->status) {
	case H2K_STATUS_VMWAIT:
		if (target->id.cpuidx < bits(long_bitmask_t)) {
			vmblock->waiting_cpus &= ~(0x1ULL << target->id.cpuidx);
		}
		H2K_ready_append(target);
		break;
	case H2K_STATUS_RUNNING:
		H2K_vm_ipi_send_withlock(target);
		break;
	case H2K_STATUS_INTBLOCKED:
		H2K_popup_cancel(target);
		H2K_ready_append(target);
		break;
	case H2K_STATUS_BLOCKED:
		H2K_futex_cancel(target);
		H2K_ready_append(target);
		break;
	case H2K_STATUS_READY:
		break;
	default:
		break;
	}
}

s32_t H2K_vmop_kill_thread(vmop_t unused0, u32_t id, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) {

	H2K_id_t target_id;
	H2K_vmblock_t *vmblock;
	H2K_thread_context *target;

	if (NULL == me) return -1;
	target_id.raw = id;
	vmblock = H2K_gp->vmblocks[target_id.vmidx];
	/* Whole VM is gone -- treat as success so callers can blindly clean up */
	if (NULL == vmblock) return 0;
	/* Same permission model as H2K_vmop_status: self-VM or child-VM */
	if (me->id.vmidx != vmblock->vmidx
			&& me->id.vmidx != vmblock->parent.vmidx) return -1;
	target = H2K_id_cpuidx_to_context(vmblock, target_id.cpuidx);
	if (NULL == target) return -1;

	BKL_LOCK();
	kill_thread_locked(vmblock, target);
	BKL_UNLOCK();
	return 0;
}

s32_t H2K_vmop_kill_vm(vmop_t unused0, u32_t vm, u32_t status, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) {

	H2K_vmblock_t *vmblock;
	u32_t i;

	if (NULL == me) return -1;
	if (vm == VMOP_KILL_VM_SELF) {
		vmblock = me->vmblock;
	} else {
		if (vm >= H2K_ID_MAX_VMS) return -1;
		vmblock = H2K_gp->vmblocks[vm];
		/* Already gone == success */
		if (NULL == vmblock) return 0;
	}
	/* Allow self-VM or parent-of-child; reject unrelated */
	if (me->id.vmidx != vmblock->vmidx
			&& me->id.vmidx != vmblock->parent.vmidx) {
		return -1;
	}

	BKL_LOCK();
	/* Stamp the desired exit status before any thread's thread_stop_withlock
	 * can read it.  Every kill-induced reap routes through H2K_vm_do_work_withlock
	 * which passes vmblock->status to thread_stop_withlock, so the value
	 * recorded here is what the parent (or sim) eventually observes. */
	vmblock->status = (s32_t)status;
	for (i = 0; i < vmblock->max_cpus; i++) {
		H2K_thread_context *target = &vmblock->contexts[i];
		kill_thread_locked(vmblock, target);
	}
	BKL_UNLOCK();
	return 0;
}
