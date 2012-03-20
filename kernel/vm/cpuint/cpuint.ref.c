/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>

#include <vm.h>
#include <cpuint.h>
#include <vmipi.h>
#include <atomic.h>
#include <q6protos.h>
#include <globals.h>
#include <hw.h>
#include <readylist.h>
#include <check_sanity.h>
#include <vmwork.h>
#include <vmevent.h>

s32_t  H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno)
{
	u32_t tmp;
	intno &= 0x1f;
	H2K_atomic_setbit(&dest->cpuint_pending,intno);
	H2K_atomic_setbit(&dest->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	tmp = dest->cpuint_pending & dest->cpuint_enabled;
	if (tmp == 0) return 0;
	H2K_vm_int_deliver(vmblock,dest,intno);
	return 0;
}

s32_t  H2K_vm_cpuint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno)
{
	u32_t bitidx   = intno & 0x1f;
	H2K_atomic_clrbit(&dest->cpuint_pending,bitidx);
	return 0;
}

s32_t  H2K_vm_cpuint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno)
{
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	/* If already enabled, return */
	if ((me->cpuint_enabled & bitmask) != 0) return 0;
	/* Else, set bit atomically */
	H2K_atomic_setbit(&me->cpuint_enabled,bitidx);
	if ((me->cpuint_pending & bitmask) != 0) {
		H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	}
	return 0;
}

s32_t  H2K_vm_cpuint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno)
{
	u32_t bitidx   = intno & 0x1f;
	u32_t bitmask  = (1)<<bitidx;
	/* If already disabled, return */
	if ((me->cpuint_enabled & bitmask) == 0) return 0;
	/* Else, clear bit atomically */
	H2K_atomic_clrbit(&me->cpuint_enabled,bitidx);
	return 0;
}

s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	int bitidx = -1;
	u32_t tmp;
	retry:
	if ((tmp = me->cpuint_enabled & me->cpuint_pending) != 0) {
		bitidx = Q6_R_ct0_R(tmp);
		if (H2K_atomic_clrbit(&me->cpuint_pending,bitidx) == 0) goto retry;
		H2K_atomic_clrbit(&me->cpuint_enabled,bitidx);
	}
	return bitidx;
}

s32_t H2K_vm_cpuint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	u32_t tmp;
	if ((tmp = me->cpuint_enabled & me->cpuint_pending) != 0) {
		return Q6_R_ct0_R(tmp);
	}
	return -1;
}

u32_t H2K_vm_cpuint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno)
{
	u32_t bitidx  = intno & 0x1f;
	u32_t ret = 0;
	ret = (me->cpuint_pending >> bitidx) & 1;
	ret |= (((me->cpuint_enabled >> bitidx) & 1) << 2);
	return ret;
}

s32_t H2K_vm_cpuint_nop()
{
	return 0;
}

s32_t H2K_vm_cpuint_maskerr(H2K_vmblock_t *vmblock, H2K_thread_context *dst, u32_t intno)
{
	return -1;
}

s32_t H2K_vm_cpuint_afferr(H2K_vmblock_t *vmblock, u32_t cpuno, u32_t intno)
{
	return -1;
}

const H2K_vm_int_ops_t H2K_vm_cpuint_ops = {
	.nop = H2K_vm_cpuint_nop,
	.post = H2K_vm_cpuint_post,
	.clear = H2K_vm_cpuint_clear,
	.enable = H2K_vm_cpuint_enable,
	.disable = H2K_vm_cpuint_disable,
	.localmask = H2K_vm_cpuint_maskerr,
	.localunmask = H2K_vm_cpuint_maskerr,
	.setaffinity = H2K_vm_cpuint_afferr,
	.get = H2K_vm_cpuint_get,
	.peek = H2K_vm_cpuint_peek,
	.status = H2K_vm_cpuint_status,
};

