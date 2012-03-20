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
#include <q6protos.h>
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

/*
 * EJP: FIXME:
 * 
 * We should reorg this so that the vmstruct has handler pointers and widths.
 * So the first one is (for example) always the cpuint and is 32 wide.
 * The next one might be shint with X interrupts wide
 * There should also exist a default handler with all nops/errors that is MAX-X wide.
 * We then just go through each handler until the end.
 * We may be able to further genericize the code for most of the enable/disable/etc
 * such that they are indices into the handler function pointer tables.  This should
 * reduce code size.
 * 
 * FIXME: For affinity / post, specify cpuid rather than index
 * FIXME: How to post interrupts securely to other VMs?
 * 
 */

IN_SECTION(".text.vm.int")
void H2K_vm_int_deliver(H2K_vmblock_t *vmblock, H2K_thread_context *thread, u32_t intno)
{
	BKL_LOCK();
	switch (thread->status) {
		case H2K_STATUS_VMWAIT:
			thread->r00 = intno;
		enqueue:
			H2K_ready_append(thread);
			H2K_check_sanity_unlock(0);
			return;
		case H2K_STATUS_RUNNING:
			if (thread->atomic_status_word & H2K_VMSTATUS_IE) {
				H2K_vm_ipi_send(thread);
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
		default: break;
	}
	BKL_UNLOCK();
}

static s32_t H2K_vm_interrupt_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	s32_t ret;
	if ((ret = H2K_vm_cpuint_peek(vmblock,me)) < 0) {
		ret = H2K_vm_shint_peek(vmblock,me);
		if (ret >= 0) ret += 32;
	};
	return ret;
}

static s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	s32_t ret;
	if ((ret = H2K_vm_cpuint_get(vmblock,me)) < 0) {
		ret = H2K_vm_shint_get(vmblock,me);
		if (ret >= 0) ret += 32;
	};
	return ret;
}

/* 5 */
void H2K_vmtrap_intop(H2K_thread_context *me)
{
	const H2K_vm_int_ops_t *ops;
	H2K_vmblock_t *vmblock = me->vmblock;
	intop_type op = (intop_type)me->r00;
	u32_t r1 = me->r01;
	u32_t r2 = me->r02;
	u32_t bad_int = 0;
	/* EJP: FIXME: doesn't work for peek/get */
	ops = (r1>=32) ? &H2K_vm_shint_ops : &H2K_vm_cpuint_ops;
	if (r1 >= 32) r1 -= 32;

	if (r1 >= vmblock->num_ints) {
		me->r00 = -1;
		bad_int = 1;
	}
	switch (op) {
	case H2K_INTOP_NOP:
		me->r00 = 0;
		return;
	case H2K_INTOP_GLOBEN:
		if (bad_int) return;
		me->r00 = ops->enable(vmblock,me,r1);
		return;
	case H2K_INTOP_GLOBDIS:
		if (bad_int) return;
		me->r00 = ops->disable(vmblock,me,r1);
		return;
	case H2K_INTOP_LOCEN:
		if (bad_int) return;
		me->r00 = ops->localunmask(vmblock,me,r1);
		return;
	case H2K_INTOP_LOCDIS:
		if (bad_int) return;
		me->r00 = ops->localmask(vmblock,me,r1);
		return;
	case H2K_INTOP_AFFINITY:
		if (bad_int) return;
		me->r00 = ops->setaffinity(vmblock,r2,r1);
		return;
	case H2K_INTOP_GET:
		me->r00 = H2K_vm_interrupt_get(vmblock,me);
		return;
	case H2K_INTOP_PEEK:
		me->r00 = H2K_vm_interrupt_peek(vmblock,me);
		return;
	case H2K_INTOP_STATUS:
		if (bad_int) return;
		me->r00 = ops->status(vmblock,me,r1);
		return;
	case H2K_INTOP_POST:
		if (bad_int) return;
		me->r00 = ops->post(vmblock,me,r1);
		return;
	case H2K_INTOP_CLEAR:
		if (bad_int) return;
		me->r00 = ops->clear(vmblock,me,r1);
		return;
	default:
		me->r00 = -1;
	}
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
