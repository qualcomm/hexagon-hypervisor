/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <id.h>
#include <vm.h>
#include <context.h>
#include <globals.h>
#include <vmint.h>

void H2K_passthru(u32_t phys_int, H2K_thread_context *unused1, u32_t unused2) {

	H2K_id_t id;
	u32_t virt_int;
	H2K_thread_context *dst;
	H2K_vmblock_t *vmblock;

	/* Get VM ID and CPU ID */
	id.raw = (u32_t)H2K_gp->inthandlers[phys_int].param;
	virt_int = id.reserved;

	/* FIXME: look in vmblock when virt_int == 0 */
	dst = H2K_id_to_context(id);
	vmblock = dst->vmblock;

	/* Ignore error from post to dead cpu */
	vmblock->intinfo[0].optab[H2K_INTOP_POST](vmblock, dst, virt_int, vmblock->intinfo);
}	
