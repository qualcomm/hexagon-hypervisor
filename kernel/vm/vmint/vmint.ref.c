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

IN_SECTION(".text.vm.int")
static s32_t H2K_vm_interrupt_deliver_cpu(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = 1<<bitidx;
	H2K_thread_context *thread;
	if (vmblock->percpu_mask[cpu][wordidx] & bitmask) {
		thread = vmblock->cpu_contexts[cpu];
		H2K_atomic_setbit(&thread->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
		if (thread->atomic_status_word & H2K_VMSTATUS_IE) {
			/* Deliver IPI */
			H2K_vm_ipi_send(thread);
		}
		
		BKL_LOCK();
		if (thread->status == H2K_STATUS_VMWAIT) { // wake up
			thread->r00 = intno; // return value from vmwait
			H2K_ready_append(thread);
			H2K_check_sanity_unlock(0);
		}
		else {
			BKL_UNLOCK();
		}
		return 1;
	} else {
		return 0;
	}
}

IN_SECTION(".text.vm.int")
static void H2K_vm_interrupt_deliver(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno)
{
	u32_t i;
	/* Look for first CPU to interrupt */
	i = first_cpu;
	do {
		if (H2K_vm_interrupt_deliver_cpu(vmblock,i,intno)) {
			return;
		} else {
			if (++i >= vmblock->num_cpus) i = 0;
		}
	} while (i != first_cpu);
	/* No CPU found to take interrupt, just return */
}

void  H2K_vm_interrupt_post(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = 1<<bitidx;
	u32_t *wptr,tmp;
	tmp = vmblock->pending[wordidx];
	/* If already pending, return */
	if (tmp & bitmask) return;
	/* Set bit atomically */
	wptr = &vmblock->pending[wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	/* Is it globally enabled? If not, finished. */
	if ((vmblock->enable[wordidx] & bitmask) == 0) return;
	H2K_vm_interrupt_deliver(vmblock,first_cpu,intno);
}

void  H2K_vm_interrupt_clear(H2K_vmblock_t *vmblock, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t *wptr;
	/* clear bit atomically */
	wptr = &vmblock->pending[wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
}

void  H2K_vm_interrupt_enable(H2K_vmblock_t *vmblock, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	/* If already enabled, return */
	if ((vmblock->enable[wordidx] & bitmask) != 0) return;
	/* Else, set bit atomically */
	wptr = &vmblock->enable[wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	if ((vmblock->pending[wordidx] & bitmask) != 0) {
		H2K_vm_interrupt_deliver(vmblock,0,intno);
	}
}

void  H2K_vm_interrupt_disable(H2K_vmblock_t *vmblock, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	/* If already disabled, return */
	if ((vmblock->enable[wordidx] & bitmask) == 0) return;
	/* Else, clear bit atomically */
	wptr = &vmblock->enable[wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
}

void  H2K_vm_interrupt_localmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	/* If already disabled, return */
	if ((vmblock->percpu_mask[cpu][wordidx] & bitmask) == 0) return;
	/* Else, clear bit atomically */
	wptr = &vmblock->percpu_mask[cpu][wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
}

void  H2K_vm_interrupt_localunmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	/* If already enabled, return */
	if ((vmblock->percpu_mask[cpu][wordidx] & bitmask) != 0) return;
	/* Else, set bit atomically */
	wptr = &vmblock->percpu_mask[cpu][wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	if ((vmblock->enable[wordidx] & vmblock->pending[wordidx] & bitmask) != 0) {
		H2K_vm_interrupt_deliver_cpu(vmblock,cpu,intno);
	}
}

void  H2K_vm_interrupt_setaffinity(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
{
	int i;
	for (i = 0; i < vmblock->num_cpus; i++) {
		H2K_vm_interrupt_localmask(vmblock,i,intno);
	}
	H2K_vm_interrupt_localunmask(vmblock,cpu,intno);
}

s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, u8_t cpu)
{
	int i,j;
	u32_t local_en;
	u32_t global_en;
	u32_t pending;
	u32_t tmp;
	bitmask_t *mycpu_masks = vmblock->percpu_mask[cpu];
	u32_t bitidx;
	for (i = j = 0; i < vmblock->num_ints; i += 32, j += 1) {
		retry:
		pending = vmblock->pending[j];
		global_en = vmblock->enable[j];
		local_en = mycpu_masks[j];
		if ((tmp = pending & global_en & local_en) != 0) {
			/* CT0(tmp) to get interrupt # */
			bitidx = Q6_R_ct0_R(tmp);
			/* Atomic Clear in Enable, if already cleared goto retry */
			if (H2K_atomic_clrbit(&vmblock->enable[j],bitidx) == 0) goto retry;
			/* Atomic Clear in Pending, if already cleared goto retry (?) */
			if (H2K_atomic_clrbit(&vmblock->pending[j],bitidx) == 0) goto retry;
			/* Return interrupt # */
			return i+bitidx;
		}
	}
	/* No interrupt found! */
	return -1;
}

s32_t H2K_vm_interrupt_peek(H2K_vmblock_t *vmblock, u8_t cpu)
{
	int i,j;
	u32_t local_en;
	u32_t global_en;
	u32_t pending;
	u32_t tmp;
	bitmask_t *mycpu_masks = vmblock->percpu_mask[cpu];
	u32_t bitidx;
	for (i = j = 0; i < vmblock->num_ints; i += 32, j += 1) {
		pending = vmblock->pending[j];
		global_en = vmblock->enable[j];
		local_en = mycpu_masks[j];
		if ((tmp = pending & global_en & local_en) != 0) {
			/* CT0(tmp) to get interrupt # */
			bitidx = Q6_R_ct0_R(tmp);
			/* Return interrupt # */
			return i+bitidx;
		}
	}
	/* No interrupt found! */
	return -1;
}

u32_t H2K_vm_interrupt_status(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
{
	u32_t wordidx = intno >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t ret = 0;
	ret = (vmblock->pending[wordidx] >> bitidx) & 1;
	ret |= (((vmblock->percpu_mask[cpu][wordidx] >> (bitidx)) & 1) << 1);
	ret |= (((vmblock->enable[wordidx] >> (bitidx)) & 1) << 2);
	return ret;
}

/* 5 */
void H2K_vmtrap_intop(H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;
	intop_type op = (intop_type)me->r00;
	u32_t r1 = me->r01;
	u32_t r2 = me->r02;
	u32_t bad_int = 0;
	u8_t mycpu = me->vmcpu;
	if (r1 >= vmblock->num_ints) {
		bad_int = 1;
		me->r00 = -1;
	} else {
		me->r00 = 0;
	}
	switch (op) {
	case H2K_INTOP_NOP:
		return;
	case H2K_INTOP_GLOBEN:
		if (bad_int) return;
		H2K_vm_interrupt_enable(vmblock,r1);
		return;
	case H2K_INTOP_GLOBDIS:
		if (bad_int) return;
		H2K_vm_interrupt_disable(vmblock,r1);
		return;
	case H2K_INTOP_LOCEN:
		if (bad_int) return;
		H2K_vm_interrupt_localunmask(vmblock,mycpu,r1);
		return;
	case H2K_INTOP_LOCDIS:
		if (bad_int) return;
		H2K_vm_interrupt_localmask(vmblock,mycpu,r1);
		return;
	case H2K_INTOP_AFFINITY:
		if (bad_int) return;
		H2K_vm_interrupt_localmask(vmblock,r2,r1);
		return;
	case H2K_INTOP_GET:
		me->r00 = H2K_vm_interrupt_get(vmblock,mycpu);
		return;
	case H2K_INTOP_PEEK:
		me->r00 = H2K_vm_interrupt_peek(vmblock,mycpu);
		return;
	case H2K_INTOP_STATUS:
		if (bad_int) return;
		me->r00 = H2K_vm_interrupt_status(vmblock,mycpu,r1);
		return;
	case H2K_INTOP_POST:
		if (bad_int) return;
		H2K_vm_interrupt_post(vmblock,mycpu,r1);
		return;
	case H2K_INTOP_CLEAR:
		if (bad_int) return;
		H2K_vm_interrupt_clear(vmblock,r1);
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
		intno = H2K_vm_interrupt_get(me->vmblock, me->vmcpu);
		if (intno != -1) {
			/* Interrupts enabled, interrupt pulled from controller.  Do interrupt! */
			H2K_vm_event(0,intno,INTERRUPT_GEVB_OFFSET,me);
		}
	} else { // interrupts disabled
		intno = H2K_vm_interrupt_peek(me->vmblock, me->vmcpu);
	}
	return intno;
}
