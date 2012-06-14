/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <vmdefs.h>
#include <vmipi.h>
#include <setjmp.h>
#include <hw.h>
#include <max.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

jmp_buf env;

H2K_thread_context a;

u32_t TH_saw_do_work;

void H2K_vm_do_work(H2K_thread_context *me)
{
	if (me != &a) FAIL("wrong thread passed as vm_do_work argument");
	TH_saw_do_work = 1;
}

#define TEST_THREAD 1

u32_t get_all_hthreads_mask()
{
	iassignw(0,-1);
	return iassignr(0);
}

#define ALL_HTHREADS_MASK get_all_hthreads_mask()

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	__asm__ __volatile(" r15 = syscfg \n r15 = clrbit(r15,#4) \n syscfg = r15 \n" : : : "r15");
	TH_saw_do_work = 0;

	/* Set up thread context */
	a.status = H2K_STATUS_RUNNING;
	a.hthread = TEST_THREAD;

	/* Setup IPI mask */
	H2K_gp->mask_for_ipi = 0;
	/* Setup interrupt masks */
	change_imask(TEST_THREAD,VM_IPI_INT);
	/* Clear interrupt from pending */
	H2K_clear_ipend(VM_IPI_INTMASK);

	/* VM IPI SEND */
	H2K_vm_ipi_send(&a);

	/* Check that mask was set */
	if ((H2K_gp->mask_for_ipi & (1<<TEST_THREAD)) == 0) FAIL("0: Didn't set mask");
	/* Check that interrupt masks were set appropriately */
	if ((get_imask(1) & VM_IPI_INTMASK) != 0) FAIL("0: Didn't unmask");
	/* Check that interrupt is pending */
	if ((H2K_get_ipend() & VM_IPI_INTMASK) == 0) FAIL("0: Didn't pend interrupt");
	
	H2K_clear_ipend(VM_IPI_INTMASK);

	/* Set up mask bits */
	H2K_gp->mask_for_ipi = 0x3;
	/* Set up interrupt masks */
	iassignw(VM_IPI_INT,~0x3);
	/* VM IPI DO */
	H2K_vm_ipi_do(VM_IPI_INT,&a,TEST_THREAD);

	/* Check that mask bit was cleared */
	if (H2K_gp->mask_for_ipi != 0x1) FAIL("a: Unexpected mask value");
	/* Check that interrupt masks were set appropriately */
	if (iassignr(VM_IPI_INT) != (ALL_HTHREADS_MASK & -2)) FAIL("a: Unexpected IMASK bits");
	/* If more mask bits are set, new interrupt should be pending */
	if ((H2K_get_ipend() & VM_IPI_INTMASK) == 0) FAIL("a: Didn't repend int");

	/* do_work now called unconditionally */
	///* check for do work If me != NULL  */
	//if (TH_saw_do_work != 0) FAIL("a: Saw call to do_work w/o VMWORK set");

	TH_saw_do_work = 0;

	H2K_clear_ipend(VM_IPI_INTMASK);

	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_IE;

	/* Set up mask bits */
	H2K_gp->mask_for_ipi = 0x3;
	/* Set up interrupt masks */
	iassignw(VM_IPI_INT,~0x3);
	/* VM IPI DO */
	H2K_vm_ipi_do(VM_IPI_INT,&a,TEST_THREAD);

	/* Check that mask bit was cleared */
	if (H2K_gp->mask_for_ipi != 0x1) FAIL("1: Unexpected mask value");
	/* Check that interrupt masks were set appropriately */
	if (iassignr(VM_IPI_INT) != (ALL_HTHREADS_MASK & -2)) FAIL("1: Unexpected IMASK bits");
	/* If more mask bits are set, new interrupt should be pending */
	if ((H2K_get_ipend() & VM_IPI_INTMASK) == 0) FAIL("1: Didn't repend int");
	/* check for do work If me != NULL  */
	if (TH_saw_do_work == 0) FAIL("1: Didn't see call to do_work");
	TH_saw_do_work = 0;

	H2K_clear_ipend(VM_IPI_INTMASK);

	/* VM IPI DO (me == NULL) */
	H2K_vm_ipi_do(VM_IPI_INT,NULL,0);
	/* Check that mask bit was cleared */
	if (H2K_gp->mask_for_ipi != 0x0) FAIL("2: Unexpected mask value");
	/* Check that interrupt masks were set appropriately */
	if (iassignr(VM_IPI_INT) != (ALL_HTHREADS_MASK)) FAIL("2: Unexpected IMASK bits");
	/* If more mask bits are set, new interrupt should be pending */
	if ((H2K_get_ipend() & VM_IPI_INTMASK) != 0) FAIL("2: Repended int");
	/* check for do work If me != NULL  */
	if (TH_saw_do_work != 0) FAIL("2: Saw call to do_work");

	puts("TEST PASSED");
	return 0;
}

