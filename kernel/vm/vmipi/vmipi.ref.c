/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <vmipi.h>
#include <globals.h>
#include <atomic.h>
#include <vmwork.h>
#include <hw.h>

void H2K_vm_ipi_do(u32_t ipi_intno, H2K_thread_context *me, u32_t hwtnum)
{
	ciad(VM_IPI_INTMASK);
	H2K_atomic_clrbit(&H2K_gp->mask_for_ipi,hwtnum);
	iassignw(VM_IPI_INT,~H2K_gp->mask_for_ipi);
	if (H2K_gp->mask_for_ipi != 0) swi(VM_IPI_INTMASK);
	if ((me != NULL) && (me->vmstatus & H2K_VMSTATUS_VMWORK)) H2K_vm_do_work(me);
}

void H2K_vm_ipi_send(H2K_thread_context *dest)
{
	BKL_LOCK(&H2K_bkl);
	if (dest->status == H2K_STATUS_RUNNING) {
		H2K_atomic_setbit(&H2K_gp->mask_for_ipi,dest->hthread);
		iassignw(VM_IPI_INT,~H2K_gp->mask_for_ipi);
		swi(VM_IPI_INTMASK);
	}
	BKL_UNLOCK(&H2K_bkl);
}

