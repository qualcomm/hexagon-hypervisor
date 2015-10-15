/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>

#include <vm.h>
#include <vmint.h>
#include <vmipi.h>
#include <atomic.h>
#include <hexagon_protos.h>
#include <globals.h>
#include <hw.h>
#include <readylist.h>
#include <check_sanity.h>
#include <vmwork.h>
#include <vmevent.h>
#include <popup.h>
#include <futex.h>
#include <shint.h>
#include <cpuint.h>
#include <badint.h>
#include <id.h>

IN_SECTION(".text.vm.int")
s32_t H2K_vm_int_deliver_locked(H2K_vmblock_t *vmblock, H2K_thread_context *thread, u32_t intno)
{
	switch (thread->status) {
	case H2K_STATUS_VMWAIT:
		if (thread->id.cpuidx < bits(long_bitmask_t)) {
			vmblock->waiting_cpus &= ~(0x1ULL << thread->id.cpuidx);
		}
		thread->r00 = intno;
	enqueue:
		H2K_ready_append(thread);
		H2K_check_sanity(0);
		return 0;
	case H2K_STATUS_RUNNING:
		if (thread->atomic_status_word & H2K_VMSTATUS_IE) {
			return H2K_vm_ipi_send_withlock(thread);
		}
		break;
	case H2K_STATUS_INTBLOCKED:
		if (thread->atomic_status_word & H2K_VMSTATUS_IE) {
			H2K_popup_cancel(thread);
			goto enqueue;
		}
		break;
	case H2K_STATUS_BLOCKED:
		if (thread->atomic_status_word & H2K_VMSTATUS_IE) {
			H2K_futex_cancel(thread);
			goto enqueue;
		}
		break;
	case H2K_STATUS_READY:
		break;
		
	case H2K_STATUS_DEAD:
	default:
		return -1;
	}
	return 0;
}

IN_SECTION(".text.vm.int")
s32_t H2K_vm_int_deliver(H2K_vmblock_t *vmblock, H2K_thread_context *thread, u32_t intno)
{
	s32_t ret;

	BKL_LOCK();
	ret = H2K_vm_int_deliver_locked(vmblock,thread,intno);
	BKL_UNLOCK();
	return ret;
}

static s32_t H2K_vm_interrupt_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	return vmblock->intinfo[0].handlers->peek(vmblock,me,0,vmblock->intinfo);
}

static s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	return vmblock->intinfo[0].handlers->get(vmblock,me,0,vmblock->intinfo);
}

/*
 * Insert here:
 * Table of xforms function pointers for preparing dst/vmblock/etc
 * Then we have everything set up for call to first int handler fns (cpuint, for now)
 * 
 * (returns dst) 
 * H2K_thread_context *fixup(H2K_thread_context *me, int *intptr);
 * 
 * Most funcs return me as dst
 * get/peek sets *intptr to 0
 * post sets *vmptr and *dstptr to dst thread
 * affinity sets ???
 * 
 */

static H2K_thread_context *H2K_vmint_fixup_getpeek(H2K_thread_context *me, u32_t *argptr)
{
	*argptr = 0;
	return me;
}

static H2K_thread_context *H2K_vmint_fixup_post(H2K_thread_context *me, u32_t *argptr)
{
	H2K_id_t id;
	H2K_thread_context *dst;
	u32_t vm;  // target ID
	H2K_vmblock_t *vmblock;  // target vmblock

	if (me->r02) {  // ID of cpu to receive interrupt
		id.raw = me->r02;
		vm = id.vmidx;

		if (vm < H2K_ID_MAX_VMS && H2K_gp->vmblocks[vm] != NULL) { // ok
			vmblock = H2K_gp->vmblocks[vm];
		} else {
			return NULL;
		}
		/* me might be NULL if called from fastint */
		if (me != NULL
				&& me->id.vmidx != vmblock->parent.vmidx  // parent -> child
				&& me->vmblock->parent.vmidx != vm        // child -> parent
				&& me->id.vmidx != vm) {                  // self
			return NULL;  // unrelated
		}

	} else {        // == 0 means me
		id = me->id;
	}

	dst = H2K_id_to_context(id);
	return dst;
}

static H2K_thread_context *H2K_vmint_fixup_aff(H2K_thread_context *me, u32_t *argptr)
{
	return H2K_id_cpuidx_to_context(me->vmblock,me->r02);
}

static H2K_thread_context *H2K_vmint_fixup_default(H2K_thread_context *me, u32_t *argptr)
{
	return me;
}

typedef H2K_thread_context *(*H2K_vmint_fixup_fn_t)(H2K_thread_context *me, u32_t *argptr);

const H2K_vmint_fixup_fn_t H2K_intops_fixups[H2K_INTOP_FIRST_INVALID_OP] = {
	H2K_vmint_fixup_default,	/* H2K_INTOP_NOP */
	H2K_vmint_fixup_default,	/* H2K_INTOP_GLOBEN */
	H2K_vmint_fixup_default,	/* H2K_INTOP_GLOBDIS */
	H2K_vmint_fixup_default,	/* H2K_INTOP_LOCEN */
	H2K_vmint_fixup_default,	/* H2K_INTOP_LOCDIS */
	H2K_vmint_fixup_aff,		/* H2K_INTOP_AFFINITY */
	H2K_vmint_fixup_getpeek,	/* H2K_INTOP_GET */
	H2K_vmint_fixup_getpeek,	/* H2K_INTOP_PEEK */
	H2K_vmint_fixup_default,	/* H2K_INTOP_STATUS */
	H2K_vmint_fixup_post,		/* H2K_INTOP_POST */
	H2K_vmint_fixup_default,	/* H2K_INTOP_CLEAR */
};
	

/* 5 */
void H2K_vmtrap_intop(H2K_thread_context *me)
{
	u32_t op = (intop_type)me->r00;
	me->r00 = -1;	/* default is fail */
	H2K_thread_context *dst;
	u32_t arg = me->r01;
	H2K_vmblock_t *vmblock;
	if (op >= H2K_INTOP_FIRST_INVALID_OP) goto fail;
	dst = H2K_intops_fixups[op](me,&arg);
	if (dst == NULL) goto fail;
	vmblock = dst->vmblock;
	me->r00 = vmblock->intinfo[0].optab[op](vmblock,dst,arg,vmblock->intinfo);
fail:
	return;
}

u32_t H2K_enable_guest_interrupts(H2K_thread_context *me) {
	u32_t prev = !(H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT));
	
	if (!prev) {  // interrupts were disabled, try to take now
			H2K_vm_check_interrupts(me);
	}
	return prev;
}

u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) {
	return H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT);
}

/* return pending unmasked interrupt and take it if enabled; -1 if none */
s32_t H2K_vm_check_interrupts(H2K_thread_context *me) {

	s32_t intno;
	
	if (me->vmstatus & H2K_VMSTATUS_IE) {
		/* Try to get interrupt */
		intno = H2K_vm_interrupt_get(me->vmblock, me);
		if (intno != -1) {
			/* Interrupts enabled, interrupt pulled from controller.  Do interrupt! */
			H2K_vm_event(0,intno,INTERRUPT_GEVB_OFFSET,me);
		}
	} else { // interrupts disabled
		intno = H2K_vm_interrupt_peek(me->vmblock, me);
	}
	return intno;
}

void H2K_vm_int_intinfo_init(H2K_vmblock_t *vmblock, u32_t num_ints)
{
	H2K_vm_int_opinfo_t *info = vmblock->intinfo;
	/* CPU INTS */
	info->num_ints = PERCPU_INTERRUPTS;
	info->handlers = &H2K_vm_cpuint_ops;
	info++;
	if (num_ints > 0) {
		info->num_ints = num_ints;
		info->handlers = &H2K_vm_shint_ops;
		info++;
	}
	info->num_ints = ~0;
	info->handlers = &H2K_vm_badint_ops;
}
