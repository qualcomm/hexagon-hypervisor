/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * There are several functions to test:
 * H2K_vm_cpuint_post
 * H2K_vm_cpuint_disable
 * H2K_vm_cpuint_enable
 * H2K_vm_cpuint_get
 * H2K_vm_cpuint_peek
 * H2K_vm_cpuint_status
 * 
 * Here is the plan:
 * + Set up a VM/CPU
 * 
 */

#define MAX_TEST_THREADS 8
#define MAX_TEST_INTERRUPTS 32

#include <c_std.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cpuint.h>
#include <vm.h>
#include <context.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

/* XXX: FIXME: set up vmblock */

H2K_vmblock_t TH_vmblock;
H2K_thread_context a;

void TH_init_vmblock()
{
	memset(&TH_vmblock,0,sizeof(TH_vmblock));
	TH_vmblock.contexts = &a;
	TH_vmblock.max_cpus = 1;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	a.status = H2K_STATUS_RUNNING;
}

u32_t TH_vmstatus_setting = 0;

u32_t TH_saw_ipi_send = 0;
u32_t TH_expected_ipi = 0;

void H2K_vm_ipi_send(H2K_thread_context *thread)
{
	TH_saw_ipi_send = 1;
}

void TH_check_ipi()
{
	if (TH_saw_ipi_send != TH_expected_ipi) {
		printf("ipi_send = %d expected = %d\n",TH_saw_ipi_send,TH_expected_ipi);
		FAIL("IPI not expected value");
	}
	TH_saw_ipi_send = 0;
}

/*
 * Reset everything 
 */
void TH_clear_data()
{
	a.cpuint_pending = 0;
	a.cpuint_enabled = 0;
}

/* Test enable / disable */
void TH_test_maskfuncs()
{
	u32_t j;
	TH_clear_data();
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		/* Test enable/disable */
		if (a.cpuint_pending) FAIL("Interrupt not cleared correctly");
		H2K_vm_cpuint_enable(&TH_vmblock,&a,j);
		if (((a.cpuint_enabled >> j) & 1) == 0) FAIL("H2K_vm_cpuint_enable");
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j);
		if (((a.cpuint_enabled >> j) & 1) != 0) FAIL("H2K_vm_cpuint_disable");
	}
}

/* Check to see if interrupt was delivered correctly */
void TH_check_delivery(u32_t intno)
{
	if (((a.cpuint_pending & a.cpuint_enabled) >> intno) & 1) {
		if (a.vmstatus & H2K_VMSTATUS_VMWORK) {
			TH_check_ipi();
		} else {
			FAIL("Didn't deliver interrupt to thread");
		}
	} else {
		if (a.vmstatus & H2K_VMSTATUS_VMWORK) {
			FAIL("Set thread to work, interrupt not enabled");
		}
	}
}

void TH_test_post(u32_t intno)
{
	a.vmstatus = TH_vmstatus_setting;
	H2K_vm_cpuint_post(&TH_vmblock,&a,intno);
	if ((a.cpuint_pending & (1<<(intno))) == 0) {
		FAIL("Did not pend interrupt");
	}
	if ((a.cpuint_enabled & (1<<(intno))) != 0) {
		TH_check_delivery(intno);
	}
}

void TH_check_enable_wakeup(u32_t intno)
{
	a.vmstatus = TH_vmstatus_setting;
	a.cpuint_pending |= 1<<intno;
	H2K_vm_cpuint_enable(&TH_vmblock,&a,intno);
	TH_check_delivery(intno);
}

void TH_test_clear(u32_t intno)
{
	a.cpuint_pending |= (1<<(intno%32));
	H2K_vm_cpuint_clear(&TH_vmblock,&a,intno);
	if ((a.cpuint_pending >> (intno)) & 1) FAIL("Didn't clear interrupt");
}

void TH_test_posting()
{
	u32_t j;
	/* Test that a normally posted interrupt will cause a thread to have VM Work */
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j);
		TH_test_post(j);
		a.cpuint_pending = 0;
		H2K_vm_cpuint_enable(&TH_vmblock,&a,j);
		TH_test_post(j);
	}
	/* Test that a disabled interrupt that is pending and gets enabled will cause VM Work */
	/* Note that for cpu interrupt, we don't need to IPI because we always enable ourselves */
	TH_expected_ipi = 0;
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j);
		TH_check_enable_wakeup(j);
	}
	/* Test that a posted interrupt can be cleared */
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		TH_test_clear(j);
	}
}

void TH_test_interrupt_get(u32_t intno)
{
	/* Starts with globally disabled / not pending */
	H2K_thread_context *me = &a;
	me->vmstatus = 0;
	/* Disabled globally / not pending */
	if (H2K_vm_cpuint_get(&TH_vmblock,me) != -1) FAIL("Got disabled/np int");
	/* Enabled globally / not pending */
	H2K_vm_cpuint_enable(&TH_vmblock,me,intno);
	if (H2K_vm_cpuint_get(&TH_vmblock,me) != -1) FAIL("Got np int");
	/* Disabled globally / pending */
	H2K_vm_cpuint_disable(&TH_vmblock,me,intno);
	a.cpuint_pending |= (1<<(intno));
	if (H2K_vm_cpuint_get(&TH_vmblock,me) != -1) FAIL("Got disabled int");
	/* Enabled globally / pending */
	a.cpuint_enabled |= (1<<(intno));
	if (H2K_vm_cpuint_get(&TH_vmblock,me) != intno) {
		FAIL("Didn't get expected interrupt");
	}
	if ((a.cpuint_enabled & (1<<(intno))) != 0) {
		FAIL("Not auto-disabled");
	}
	if ((a.cpuint_pending & (1<<(intno))) != 0) {
		FAIL("Not cleared from pending");
	}
	H2K_vm_cpuint_disable(&TH_vmblock,me,intno);
	a.cpuint_pending = 0;
}

void TH_test_status()
{
	int i;
	u32_t tmp;
	a.cpuint_enabled = 0;
	a.cpuint_pending = 0;
	for (i = 0; i < MAX_TEST_INTERRUPTS; i++) {
		if (i & 4) a.cpuint_enabled |= (1<<(i));
		if (i & 1) a.cpuint_pending |= (1<<(i));
	}
	for (i = 0; i < MAX_TEST_INTERRUPTS; i++) {
		tmp = H2K_vm_cpuint_status(&TH_vmblock,&a,i);
		if (tmp != (i & 0x5)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7)));
			FAIL("Bad bits");
		}
	}
}

int main()
{
	int j;
	TH_init_vmblock();

	/* Check status */
	TH_test_status();

	/* int_v2p not set up */
	/* pmap not set up */
	/* Set set/clear mask functions basic functionality */
	TH_test_maskfuncs();

	/* Set up for interrupt disabled checks */
	TH_vmstatus_setting = 0;
	TH_expected_ipi = 0;
	TH_test_posting();
	puts("a");

	/* Set up for interrupt enabled checks */
	TH_vmstatus_setting = H2K_VMSTATUS_IE;
	TH_expected_ipi = 1;
	TH_test_posting();
	puts("b");

	/* Test interrupt get */
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	a.cpuint_pending = 0;
	a.cpuint_enabled = 0;
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		TH_test_interrupt_get(j);
	}
	puts("c");

	/* Further get testing */

	a.cpuint_pending = 0xffffffff;
	a.cpuint_enabled = 0xffffffff;
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a) != 0) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	if (H2K_vm_cpuint_get(&TH_vmblock,&a) != 0) {
		FAIL("Didn't get first valid interrupt/get");
	}
	if (a.cpuint_pending != 0xfffffffe) {
		FAIL("Didn't clear pending bit");
	}
	if (a.cpuint_enabled != 0xfffffffe) {
		FAIL("Didn't clear enable bit");
	}
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a) != 1) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	a.cpuint_pending = 0;
	TH_vmblock.num_ints = 32;
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (H2K_vm_cpuint_get(&TH_vmblock,&a) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	/* OK!  We're done here! */
	puts("TEST PASSED");
	return 0;
}

