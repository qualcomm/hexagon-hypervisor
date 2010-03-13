/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <c_std.h>

#include <vm.h>
#include <vmint.h>
#include <vmipi.h>
#include <atomic.h>
#include <q6protos.h>
#include <globals.h>

s32_t H2K_vm_interrupt_deliver_cpu(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)
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
		return 1;
	} else {
		return 0;
	}
}

void H2K_vm_interrupt_deliver(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno)
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

