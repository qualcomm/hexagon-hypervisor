/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>

#include <vm.h>
#include <vmint.h>
#include <shint.h>
#include <vmipi.h>
#include <atomic.h>
#include <hexagon_protos.h>
#include <globals.h>
#include <hw.h>
#include <readylist.h>
#include <check_sanity.h>
#include <vmwork.h>
#include <vmevent.h>
#include <intcontrol.h>

#define CHECK_FOR_CHAIN(op) \
	do { \
		u32_t _num_ints_ = info->num_ints; \
		if (intno >= _num_ints_) { \
			info++; \
			return info->handlers->op(vmblock,me,intno-_num_ints_,info); \
		} \
	} while (0)

IN_SECTION(".text.vm.int")
static s32_t H2K_vm_shint_deliver_cpu(H2K_vmblock_t *vmblock, H2K_thread_context *dest, 
	u32_t intno)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = 1<<bitidx;
	u32_t cpu = dest->id.cpuidx;
	s32_t ret;

	if (vmblock->percpu_mask[cpu][wordidx] & bitmask) {
		H2K_atomic_setbit(&dest->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);

		/* Might get an error from deliver if the cpu dies here */
		ret = H2K_vm_int_deliver(vmblock,dest,intno);
		if (ret == -1) {
			return 0;  // keep trying
		} else {
			return 1;
		}
	}
	return 0;
}

IN_SECTION(".text.vm.int")
static void H2K_vm_shint_deliver(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno)
{
	u32_t i;
	/* FIXME: what is cpuidx in fastint context?  Always 0? */
	u32_t cpu = me->id.cpuidx;

	/* start with first waiting cpu */
	/* we only check the first 64 cpus */
	i = Q6_R_ct0_P(vmblock->waiting_cpus);

	/* Have to check all the way up to max_cpus since IDs aren't contiguous */
	for (; i < bits(long_bitmask_t) && (i < vmblock->max_cpus); i++) {
		if ((vmblock->waiting_cpus >> i) & 0x1) { // found
			if (H2K_vm_shint_deliver_cpu(vmblock, &vmblock->contexts[i], intno)) {
				return;
			}
		}
	}

	/* No luck with waiters; check them all now */
	i = cpu;
	do {
		if (H2K_vm_shint_deliver_cpu(vmblock,&vmblock->contexts[i],intno)) {
			return;
		} else {
			if (++i >= vmblock->max_cpus) i = 0;
		}
	} while (i != cpu);
	/* No CPU found to take interrupt, just return */
}

s32_t  H2K_vm_shint_post(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = 1<<bitidx;
	u32_t *wptr,tmp;
	CHECK_FOR_CHAIN(post);
	tmp = vmblock->pending[wordidx];
	/* If already pending, return */
	if (tmp & bitmask) return 0;
	/* Set bit atomically */
	wptr = &vmblock->pending[wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	/* Is it globally enabled? If not, finished. */
	if ((vmblock->enable[wordidx] & bitmask) == 0) return 0;
	H2K_vm_shint_deliver(vmblock,me,intno);
	return 0;
}

s32_t  H2K_vm_shint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t *wptr;
	CHECK_FOR_CHAIN(clear);
	/* clear bit atomically */
	wptr = &vmblock->pending[wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
	return 0;
}

s32_t  H2K_vm_shint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	CHECK_FOR_CHAIN(enable);
	/* If already enabled, return */
	if ((vmblock->enable[wordidx] & bitmask) != 0) return 0;
	/* Else, set bit atomically */
	wptr = &vmblock->enable[wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	if (vmblock->int_v2p != NULL && vmblock->int_v2p[intno + PERCPU_INTERRUPTS]) { // mapped, re-enable phys
		H2K_intcontrol_enable(vmblock->int_v2p[intno + PERCPU_INTERRUPTS]);
	}
	if ((vmblock->pending[wordidx] & bitmask) != 0) {
		H2K_vm_shint_deliver(vmblock,me,intno);
	}
	return 0;
}

s32_t  H2K_vm_shint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	CHECK_FOR_CHAIN(disable);
	/* If already disabled, return */
	if ((vmblock->enable[wordidx] & bitmask) == 0) return 0;
	/* Else, clear bit atomically */
	wptr = &vmblock->enable[wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
	return 0;
}

s32_t  H2K_vm_shint_localen(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	u32_t cpu = me->id.cpuidx;
	CHECK_FOR_CHAIN(localen);
	/* If already enabled, return */
	if ((vmblock->percpu_mask[cpu][wordidx] & bitmask) != 0) return 0;
	/* Else, set bit atomically */
	wptr = &vmblock->percpu_mask[cpu][wordidx];
	H2K_atomic_setbit(wptr,bitidx);
	if ((vmblock->enable[wordidx] & vmblock->pending[wordidx] & bitmask) != 0) {
		H2K_vm_shint_deliver_cpu(vmblock,me,intno);
	}
	return 0;
}

s32_t  H2K_vm_shint_localdis(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	u32_t *wptr;
	u32_t cpu = me->id.cpuidx;
	CHECK_FOR_CHAIN(localdis);
	/* If already disabled, return */
	if ((vmblock->percpu_mask[cpu][wordidx] & bitmask) == 0) return 0;
	/* Else, clear bit atomically */
	wptr = &vmblock->percpu_mask[cpu][wordidx];
	H2K_atomic_clrbit(wptr,bitidx);
	return 0;
}

s32_t  H2K_vm_shint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	int i;
	CHECK_FOR_CHAIN(setaffinity);
	for (i = 0; i < vmblock->max_cpus; i++) {
		H2K_vm_shint_localdis(vmblock,&vmblock->contexts[i],intno,info);
	}
	H2K_vm_shint_localen(vmblock,&vmblock->contexts[me->id.cpuidx],intno,info);
	return 0;
}

s32_t H2K_vm_shint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)
{
	int i,j;
	u32_t local_en;
	u32_t global_en;
	u32_t pending;
	u32_t tmp;
	u32_t cpu = me->id.cpuidx;
	bitmask_t *mycpu_masks = vmblock->percpu_mask[cpu];
	u32_t bitidx;
	u32_t num_ints = vmblock->num_ints;
	for (i = j = 0; i < num_ints; i += 32, j += 1) {
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
			return offset+i+bitidx;
		}
	}
	/* No interrupt found! */
	info++;
	return info->handlers->get(vmblock,me,offset+num_ints,info);
}

s32_t H2K_vm_shint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)
{
	int i,j;
	u32_t local_en;
	u32_t global_en;
	u32_t pending;
	u32_t tmp;
	u32_t cpu = me->id.cpuidx;
	bitmask_t *mycpu_masks = vmblock->percpu_mask[cpu];
	u32_t bitidx;
	u32_t num_ints = vmblock->num_ints;
	for (i = j = 0; i < num_ints; i += 32, j += 1) {
		pending = vmblock->pending[j];
		global_en = vmblock->enable[j];
		local_en = mycpu_masks[j];
		if ((tmp = pending & global_en & local_en) != 0) {
			/* CT0(tmp) to get interrupt # */
			bitidx = Q6_R_ct0_R(tmp);
			/* Return interrupt # */
			return offset+i+bitidx;
		}
	}
	/* No interrupt found! */
	info++;
	return info->handlers->peek(vmblock,me,offset+num_ints,info);
}

s32_t H2K_vm_shint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t wordidx = (intno) >> 5;
	u32_t bitidx   = intno & 0x1f;
	u32_t ret = 0;
	u32_t cpu = me->id.cpuidx;
	CHECK_FOR_CHAIN(status);
	ret = (vmblock->pending[wordidx] >> bitidx) & 1;
	ret |= (((vmblock->percpu_mask[cpu][wordidx] >> (bitidx)) & 1) << 1);
	ret |= (((vmblock->enable[wordidx] >> (bitidx)) & 1) << 2);
	return ret;
}

s32_t H2K_vm_shint_nop(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	return 0;
}

const H2K_vm_int_ops_t H2K_vm_shint_ops = {
	.nop = H2K_vm_shint_nop,
	.enable = H2K_vm_shint_enable,
	.disable = H2K_vm_shint_disable,
	.localen = H2K_vm_shint_localen,
	.localdis = H2K_vm_shint_localdis,
	.setaffinity = H2K_vm_shint_setaffinity,
	.get = H2K_vm_shint_get,
	.peek = H2K_vm_shint_peek,
	.status = H2K_vm_shint_status,
	.post = H2K_vm_shint_post,
	.clear = H2K_vm_shint_clear,
};

