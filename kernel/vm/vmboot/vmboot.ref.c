/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>
#include <globals.h>
#include <idtype.h>

s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me)
{
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

u32_t H2K_vmstatus(u32_t vm, H2K_thread_context *me) {

	H2K_vmblock_t *vmblock;

	/* Can only query child vm */
	if (vm < H2K_ID_MAX_VMS && H2K_gp->vmblocks[vm] != NULL) { // ok
		vmblock = H2K_gp->vmblocks[vm];
		if (me != NULL && me->id.vmidx != vmblock->parent.vmidx) return 0;
	} else {
		return 0;
	}

	return vmblock->status;
}
