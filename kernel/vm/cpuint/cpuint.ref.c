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
#include <hexagon_protos.h>
#include <globals.h>
#include <hw.h>
#include <readylist.h>
#include <check_sanity.h>
#include <vmwork.h>
#include <vmevent.h>
#include <intcontrol.h>
#include <max.h>

#define CHECK_FOR_CHAIN(op) \
	do { \
		if (intno >= PERCPU_INTERRUPTS) { \
			info++; \
			return info->handlers->op(vmblock, me, intno - PERCPU_INTERRUPTS, info); \
		} \
	} while (0)

/* Note that "me" is actually "dst", but make it uniform */
s32_t  H2K_vm_cpuint_post_locked(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t tmp;
	H2K_atomic_setbit(&me->cpuint_enabled_pending,intno);
	H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	tmp = me->cpuint_pending & me->cpuint_enabled;
	if (tmp == 0) return 0;
	/* Ignore error from post to dead cpu */
	H2K_vm_int_deliver_locked(vmblock,me,intno);
	return 0;
}

/* Note that "me" is actually "dst", but make it uniform */
s32_t  H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t tmp;
	CHECK_FOR_CHAIN(post);
	H2K_atomic_setbit(&me->cpuint_enabled_pending,intno);
	H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	tmp = me->cpuint_pending & me->cpuint_enabled;
	if (tmp == 0) return 0;
	/* Ignore error from post to dead cpu */
	H2K_vm_int_deliver(vmblock,me,intno);
	return 0;
}

s32_t  H2K_vm_cpuint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t bitidx   = intno;
	CHECK_FOR_CHAIN(clear);
	H2K_atomic_clrbit(&me->cpuint_enabled_pending,bitidx);
	return 0;
}

s32_t  H2K_vm_cpuint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t bitidx   = intno;
	u32_t bitmask  = (1)<<bitidx;
	CHECK_FOR_CHAIN(enable);
	/* If already enabled, return */
	if ((me->cpuint_enabled & bitmask) != 0) return 0;
	/* Else, set bit atomically */
	H2K_atomic_setbit(&me->cpuint_enabled_pending,bitidx+16);
	if (vmblock->int_v2p != NULL && vmblock->int_v2p[intno]) { // mapped, re-enable phys
		H2K_intcontrol_enable(vmblock->int_v2p[intno]);
	}
	if ((me->cpuint_pending & bitmask) != 0) {
		H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
	}
	return 0;
}

s32_t  H2K_vm_cpuint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t bitidx   = intno;
	u32_t bitmask  = (1)<<bitidx;
	CHECK_FOR_CHAIN(disable);
	/* If already disabled, return */
	if ((me->cpuint_enabled & bitmask) == 0) return 0;
	/* Else, clear bit atomically */
	H2K_atomic_clrbit(&me->cpuint_enabled_pending,bitidx+16);
	return 0;
}

s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t offset, H2K_vm_int_opinfo_t *info)
{
	int bitidx;
	u32_t tmp;
	retry:
	if ((tmp = me->cpuint_enabled & me->cpuint_pending) != 0) {
		bitidx = Q6_R_ct0_R(tmp);
		if (H2K_atomic_clrbit(&me->cpuint_enabled_pending,bitidx) == 0) goto retry;
		H2K_atomic_clrbit(&me->cpuint_enabled_pending,bitidx+16);
		return offset+bitidx;
	} else {
		info++;
		return info->handlers->get(vmblock,me,offset+PERCPU_INTERRUPTS,info);
	}
}

s32_t H2K_vm_cpuint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t offset, H2K_vm_int_opinfo_t *info)
{
	u32_t tmp;
	if ((tmp = me->cpuint_enabled & me->cpuint_pending) != 0) {
		return offset+Q6_R_ct0_R(tmp);
	}
	info++;
	return info->handlers->peek(vmblock,me,offset+PERCPU_INTERRUPTS,info);
}

s32_t H2K_vm_cpuint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	u32_t bitidx  = intno;
	s32_t ret = 0;
	CHECK_FOR_CHAIN(status);
	ret = (me->cpuint_pending >> bitidx) & 1;
	ret |= (((me->cpuint_enabled >> bitidx) & 1) << 2);
	return ret;
}

s32_t H2K_vm_cpuint_nop(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, 
	H2K_vm_int_opinfo_t *info)
{
	return 0;
}

s32_t H2K_vm_cpuint_localen(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	CHECK_FOR_CHAIN(localen);
	return -1;
}

s32_t H2K_vm_cpuint_localdis(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	CHECK_FOR_CHAIN(localdis);
	return -1;
}

s32_t H2K_vm_cpuint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, 
	u32_t intno, H2K_vm_int_opinfo_t *info)
{
	CHECK_FOR_CHAIN(setaffinity);
	return -1;
}

const H2K_vm_int_ops_t H2K_vm_cpuint_ops = {
	.nop = H2K_vm_cpuint_nop,
	.enable = H2K_vm_cpuint_enable,
	.disable = H2K_vm_cpuint_disable,
	.localen = H2K_vm_cpuint_localen,
	.localdis = H2K_vm_cpuint_localdis,
	.setaffinity = H2K_vm_cpuint_setaffinity,
	.get = H2K_vm_cpuint_get,
	.peek = H2K_vm_cpuint_peek,
	.status = H2K_vm_cpuint_status,
	.post = H2K_vm_cpuint_post,
	.clear = H2K_vm_cpuint_clear,
};

