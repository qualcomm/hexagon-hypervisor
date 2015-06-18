/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>
#include <globals.h>
#include <idtype.h>
#include <alloc.h>
#include <vmop.h>
#include <hw.h>

typedef s32_t (*vmop_ptr_t)(vmop_t, u32_t, u32_t, u32_t, u32_t, u32_t, H2K_thread_context *);

static const vmop_ptr_t H2K_vmoptab[VMOP_MAX] IN_SECTION(".data.config.config") = {
	H2K_vmop_boot,
	H2K_vmop_status,
	H2K_vmop_free
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
