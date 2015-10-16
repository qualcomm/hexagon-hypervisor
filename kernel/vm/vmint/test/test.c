/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * There are several functions to test:
 * 
 * H2K_vm_int_deliver
 * H2K_vm_interrupt_peek
 * H2K_vm_interrupt_get
 * H2K_vm_trap_intop
 * H2K_vmtrap_intop
 * H2K_enable_guest_interrupts
 * H2K_disable_guest_interrupts 
 * H2K_vm_check_interrupts
 * 
 * These functions make use of cpuint/shint/... calls and ops structures
 * 
 * 
 * Here is the plan:
 * + Use cpuint / shint
 * + Set up interrupt scenarios, make sure expected behavior happens
 * 
 */

#define MAX_TEST_THREADS 8
#define MAX_TEST_INTERRUPTS 512

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <vmint.h>
#include <shint.h>
#include <cpuint.h>
#include <badint.h>
#include <vm.h>
#include <hw.h>
#include <context.h>
#include <string.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context TH_threads[MAX_TEST_THREADS];
u32_t TH_pending_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_enable_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_localmask_storage[MAX_TEST_THREADS][MAX_TEST_INTERRUPTS/32];
u32_t *TH_localmask_ptr_storage[MAX_TEST_THREADS];
H2K_vm_int_opinfo_t TH_intinfo[3];

H2K_vmblock_t TH_vmblock;

H2K_kg_t H2K_kg;

void TH_init_vmblock()
{
	int i,j;
	memset(&TH_vmblock,0,sizeof(TH_vmblock));
	TH_vmblock.contexts = &TH_threads[0];
	TH_vmblock.max_cpus = MAX_TEST_THREADS;
	TH_vmblock.parent.vmidx = 1; // parent is boot VM
	//TH_vmblock.num_cpus = MAX_TEST_THREADS;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	TH_vmblock.pending = &TH_pending_storage[0];
	TH_vmblock.enable = &TH_enable_storage[0];
	TH_vmblock.percpu_mask = &TH_localmask_ptr_storage[0];
	TH_vmblock.intinfo = TH_intinfo;
	for (i = 0; i < MAX_TEST_INTERRUPTS/32; i++) {
		TH_pending_storage[i] = 0;
		TH_enable_storage[i] = 0;
	}
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		memset(&TH_threads[i],0,sizeof(TH_threads[i]));
		TH_threads[i].id.cpuidx = i;
		TH_threads[i].id.vmidx = 2;
		TH_threads[i].status = H2K_STATUS_RUNNING;
		TH_threads[i].vmblock = &TH_vmblock;
		TH_localmask_ptr_storage[i] = &TH_localmask_storage[i][0];
		for (j = 0; j < MAX_TEST_INTERRUPTS/32; j++) {
			TH_localmask_storage[i][j] = 0;
		}
	}
	TH_intinfo[0].num_ints = 32;
	TH_intinfo[0].handlers = &H2K_vm_cpuint_ops;
	TH_intinfo[1].num_ints = MAX_TEST_INTERRUPTS;
	TH_intinfo[1].handlers = &H2K_vm_shint_ops;
	TH_intinfo[2].num_ints = ~0;
	TH_intinfo[2].handlers = &H2K_vm_badint_ops;
	H2K_kg.vmblocks[2] = &TH_vmblock;
}

u8_t primes[MAX_TEST_THREADS] = { 2, 3, 5, 7, 11, 13, 17, 19 };

u32_t TH_vmstatus_setting = 0;

u32_t TH_saw_ipi_send = 0;
u32_t TH_expected_ipi = 0;

int H2K_vm_ipi_send_withlock(H2K_thread_context *thread)
{
	TH_saw_ipi_send = 1;
	return 0;
}

u32_t TH_expected_event = 0;
u32_t TH_saw_event = 0;
void H2K_vm_event(u32_t x, u32_t cause, u32_t offset, H2K_thread_context *me)
{
	if (TH_expected_event == 0) {
		printf("arg: %x/%u cause: %x/%u offset=%x/%u\n",x,x,cause,cause,offset,offset);
		FAIL("Unexpected event!");
	}
	TH_saw_event = 1;
}

u32_t TH_expected_sanity = 0;
u32_t TH_saw_sanity = 0;

u64_t H2K_check_sanity_unlock(u64_t ret)
{
	if (TH_expected_sanity == 0) FAIL("Didn't expect sanity");
	TH_saw_sanity = 1;
	BKL_UNLOCK();
	return ret;
}

u64_t H2K_check_sanity(u64_t ret)
{
	if (TH_expected_sanity == 0) FAIL("Didn't expect sanity");
	TH_saw_sanity = 1;
	return ret;
}

u32_t TH_expected_popup_cancel = 0;
u32_t TH_saw_popup_cancel = 0;
void H2K_popup_cancel(H2K_thread_context *me)
{
	if (TH_expected_popup_cancel == 0) {
		FAIL("Unexpected popup_cancel!");
	}
	TH_saw_popup_cancel = 1;
}

u32_t TH_expected_futex_cancel = 0;
u32_t TH_saw_futex_cancel = 0;
void H2K_futex_cancel(H2K_thread_context *me)
{
	if (TH_expected_futex_cancel == 0) {
		FAIL("Unexpected futex_cancel!");
	}
	TH_saw_futex_cancel = 1;
}

void TH_check_ipi()
{
	if (TH_saw_ipi_send != TH_expected_ipi) FAIL("IPI not expected value");
	TH_saw_ipi_send = 0;
}

void TH_setup_intmask()
{
	u32_t i,j;
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		for (j = 0; j < MAX_TEST_INTERRUPTS; j += primes[i]) {
			TH_localmask_storage[i][j/32] |= (1<<(j % 32));
		}
	}
}

void TH_clear_data()
{
	u32_t i,j;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	for (j = 0; j < MAX_TEST_INTERRUPTS; j+=32) {
		TH_pending_storage[j/32] = 0;
		TH_enable_storage[j/32] = 0;
		for (i = 0; i < MAX_TEST_THREADS; i++) {
			TH_localmask_storage[i][j/32] = 0;
		}
	}
}

int main()
{
	int i,j,k;
	s32_t ret;
	TH_init_vmblock();
	/* Set up post/get test masks */
	TH_setup_intmask();
	TH_clear_data();
	H2K_thread_context *t0 = &TH_threads[0];
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	puts("VM enable/disable/check");

	puts("A");
	if (H2K_disable_guest_interrupts(t0) != 0) FAIL("Bad disable return (!0)");
	if (t0->vmstatus & H2K_VMSTATUS_IE) FAIL("Set IE bit");
	t0->vmstatus |= H2K_VMSTATUS_IE;
	if (H2K_disable_guest_interrupts(t0) != 1) FAIL("Bad disable return (!1)");
	if (t0->vmstatus & H2K_VMSTATUS_IE) FAIL("Didn't clear IE bit");

	puts("B");
	t0->vmstatus |= H2K_VMSTATUS_IE;
	if (H2K_enable_guest_interrupts(t0) != 1) FAIL("Bad enable return (!1)");
	if ((t0->vmstatus & H2K_VMSTATUS_IE) == 0) FAIL("cleared IE bit");
	t0->vmstatus = 0;
	if (H2K_enable_guest_interrupts(t0) != 0) FAIL("Bad enable return (!0)");
	if ((t0->vmstatus & H2K_VMSTATUS_IE) == 0) FAIL("Didn't set IE bit");

	puts("C");
	t0->vmstatus = 0;
	if (H2K_vm_check_interrupts(t0) != -1) FAIL("check interrupts returned an int");

	puts("D");
	t0->cpuint_enabled = 1;
	t0->cpuint_pending = 1;
	if (H2K_vm_check_interrupts(t0) != 0) FAIL("check interrupts peek failed");

	puts("E");
	t0->cpuint_enabled = 2;
	t0->cpuint_pending = 2;
	if (H2K_vm_check_interrupts(t0) != 1) FAIL("check interrupts peek failed");

	puts("F");
	t0->cpuint_enabled = 0;
	t0->cpuint_pending = 0;
	TH_vmblock.pending[0] = 1;
	TH_vmblock.enable[0] = 1;
	TH_vmblock.percpu_mask[0][0] = 1;
	if (H2K_vm_check_interrupts(t0) != PERCPU_INTERRUPTS) FAIL("check interrupts peek failed");

	puts("G");
	t0->cpuint_enabled = 0;
	t0->cpuint_pending = 0;
	TH_vmblock.pending[0] = 0;
	TH_vmblock.enable[0] = 0;
	if (H2K_vm_check_interrupts(t0) != -1) FAIL("check interrupts peek failed");

	puts("H");
	t0->vmstatus = H2K_VMSTATUS_IE;
	if (H2K_vm_check_interrupts(t0) != -1) FAIL("check interrupts returned an intr");
	if (TH_saw_event != 0) FAIL("Saw unexpected event");

	puts("I");
	TH_expected_event = 1;

	t0->cpuint_enabled = 1;
	t0->cpuint_pending = 1;
	if (H2K_vm_check_interrupts(t0) != 0) FAIL("check interrupts get failed");
	if (t0->cpuint_enabled || t0->cpuint_pending) FAIL("get didn't work right");
	if (TH_saw_event != 1) FAIL("Didn't see event");
	TH_saw_event = 0;

	puts("J");
	t0->cpuint_enabled = 2;
	t0->cpuint_pending = 2;
	if (H2K_vm_check_interrupts(t0) != 1) FAIL("check interrupts get failed");
	if (t0->cpuint_enabled || t0->cpuint_pending) FAIL("get didn't work right");
	if (TH_saw_event != 1) FAIL("Didn't see event");
	TH_saw_event = 0;

	puts("K");
	t0->cpuint_enabled = 0;
	t0->cpuint_pending = 0;
	TH_vmblock.pending[0] = 1;
	TH_vmblock.enable[0] = 1;
	if (H2K_vm_check_interrupts(t0) != PERCPU_INTERRUPTS) FAIL("check interrupts get failed");
	if (TH_saw_event != 1) FAIL("Didn't see event");
	if (TH_vmblock.pending[0] || TH_vmblock.enable[0]) {
		printf("pending: %x enable: %x\n",TH_vmblock.pending[0],TH_vmblock.enable[0]);
		FAIL("get didn't work right");
	}
	TH_saw_event = 0;

	puts("L");
	TH_expected_event = 0;
	t0->cpuint_enabled = 0;
	t0->cpuint_pending = 0;
	TH_vmblock.pending[0] = 0;
	TH_vmblock.enable[0] = 0;
	if (H2K_vm_check_interrupts(t0) != -1) FAIL("check interrupts get failed");
	if (TH_saw_event == 1) FAIL("Saw event");
	TH_saw_event = 0;

	puts("M");
	t0->vmstatus |= H2K_VMSTATUS_IE;
	TH_expected_event = 0;
	t0->cpuint_enabled = 1;
	t0->cpuint_pending = 1;
	if (H2K_enable_guest_interrupts(t0) != 1) FAIL("Enable failed");
	if (TH_saw_event == 1) FAIL("Saw event");
	if ((t0->cpuint_enabled != 1) || (t0->cpuint_pending != 1)) FAIL("took int?");
	TH_saw_event = 0;

	puts("N");
	t0->vmstatus = 0;
	TH_expected_event = 1;
	t0->cpuint_enabled = 1;
	t0->cpuint_pending = 1;
	if (H2K_enable_guest_interrupts(t0) != 0) FAIL("Enable failed");
	if (TH_saw_event != 1) FAIL("Didn't see event");
	if ((t0->cpuint_enabled != 0) || (t0->cpuint_pending != 0)) FAIL("took int?");
	TH_saw_event = 0;

	puts("VM int deliver");
	/* DEAD, RUNNING, READY, BLOCKED, VMWAIT, INTBLOCKED */

	puts("A");
	t0->vmstatus = 0;
	t0->status = H2K_STATUS_DEAD;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != -1) FAIL("error code");
	if (t0->status != H2K_STATUS_DEAD) FAIL("status");
	if (t0->vmstatus != 0) FAIL("vmstatus");
	

	puts("B");
	t0->vmstatus = 0;
	t0->status = H2K_STATUS_VMWAIT;
	TH_expected_sanity = 1;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_READY) FAIL("status");
	if (t0->vmstatus != 0) FAIL("vmstatus");
	if (TH_saw_sanity != 1) FAIL("no sanity check");
	TH_saw_sanity = 0;
	TH_expected_sanity = 0;
	

	puts("C");
	t0->vmstatus = 0;
	t0->status = H2K_STATUS_RUNNING;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_RUNNING) FAIL("status");
	if (t0->vmstatus != 0) FAIL("vmstatus");
	

	puts("D");
	t0->vmstatus = H2K_VMSTATUS_IE;
	t0->status = H2K_STATUS_RUNNING;
	TH_expected_ipi = 1;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_RUNNING) FAIL("status");
	if (t0->vmstatus != H2K_VMSTATUS_IE) FAIL("vmstatus");
	if (TH_saw_ipi_send != 1) FAIL("Didn't see IPI");
	TH_expected_ipi = 0;
	TH_saw_ipi_send = 0;
	

	puts("E");
	t0->vmstatus = 0;
	t0->status = H2K_STATUS_INTBLOCKED;
	TH_expected_popup_cancel = 0;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_INTBLOCKED) FAIL("status");
	if (t0->vmstatus != 0) FAIL("vmstatus");
	if (TH_saw_popup_cancel != 0) FAIL("saw popup cancel");
	TH_expected_popup_cancel = 0;
	TH_saw_popup_cancel = 0;
	

	puts("F");
	t0->vmstatus = H2K_VMSTATUS_IE;
	t0->status = H2K_STATUS_INTBLOCKED;
	TH_expected_popup_cancel = 1;
	TH_expected_sanity = 1;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_READY) FAIL("status");
	if (t0->vmstatus != H2K_VMSTATUS_IE) FAIL("vmstatus");
	if (TH_saw_popup_cancel != 1) FAIL("Didn't see popup cancel");
	TH_expected_popup_cancel = 0;
	TH_saw_popup_cancel = 0;
	if (TH_saw_sanity != 1) FAIL("no sanity check");
	TH_saw_sanity = 0;
	TH_expected_sanity = 0;
	

	puts("G");
	t0->vmstatus = 0;
	t0->status = H2K_STATUS_BLOCKED;
	TH_expected_futex_cancel = 0;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_BLOCKED) FAIL("status");
	if (t0->vmstatus != 0) FAIL("vmstatus");
	if (TH_saw_futex_cancel != 0) FAIL("saw popup cancel");
	TH_expected_futex_cancel = 0;
	TH_saw_futex_cancel = 0;
	

	puts("H");
	t0->vmstatus = H2K_VMSTATUS_IE;
	t0->status = H2K_STATUS_BLOCKED;
	TH_expected_futex_cancel = 1;
	TH_expected_sanity = 1;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_READY) FAIL("status");
	if (t0->vmstatus != H2K_VMSTATUS_IE) FAIL("vmstatus");
	if (TH_saw_futex_cancel != 1) FAIL("Didn't see popup cancel");
	TH_expected_futex_cancel = 0;
	TH_saw_futex_cancel = 0;
	if (TH_saw_sanity != 1) FAIL("no sanity check");
	TH_saw_sanity = 0;
	TH_expected_sanity = 0;
	

	puts("I");
	t0->vmstatus = H2K_VMSTATUS_IE;
	t0->status = H2K_STATUS_READY;
	ret = H2K_vm_int_deliver(&TH_vmblock,t0,0);
	if (ret != 0) FAIL("error code");
	if (t0->status != H2K_STATUS_READY) FAIL("status");
	if (t0->vmstatus != H2K_VMSTATUS_IE) FAIL("vmstatus");

	puts("intop handler");

	puts("A");
	t0->r00 = H2K_INTOP_NOP;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("nop");

	puts("B");
	t0->cpuint_enabled = 0;
	TH_vmblock.enable[0] = 0;
	t0->r00 = H2K_INTOP_GLOBEN;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable0 ret");
	if (t0->cpuint_enabled != 1) FAIL("enable0 beh");

	t0->r00 = H2K_INTOP_GLOBEN;
	t0->r01 = 1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable1 ret");
	if (t0->cpuint_enabled != 3) {
		printf("val: %x\n",t0->cpuint_enabled);
		FAIL("enable1 beh");
	}

	t0->r00 = H2K_INTOP_GLOBEN;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable32 ret");
	if (TH_vmblock.enable[0] != 1) {
		printf("val: %x\n",TH_vmblock.enable[0]);
		FAIL("enable32 beh");
	}

	t0->r00 = H2K_INTOP_GLOBEN;
	t0->r01 = PERCPU_INTERRUPTS+1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable33 ret");
	if (TH_vmblock.enable[0] != 3) FAIL("enable33 beh");

	t0->r00 = H2K_INTOP_GLOBEN;
	t0->r01 = 1026;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("oob ret");

	puts("C");
	t0->cpuint_enabled = 3;
	TH_vmblock.enable[0] = 3;
	t0->r00 = H2K_INTOP_GLOBDIS;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable0 ret");
	if (t0->cpuint_enabled != 2) FAIL("disable0 beh");

	t0->r00 = H2K_INTOP_GLOBDIS;
	t0->r01 = 1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable1 ret");
	if (t0->cpuint_enabled != 0) {
		printf("val: %x\n",t0->cpuint_enabled);
		FAIL("disable1 beh");
	}

	t0->r00 = H2K_INTOP_GLOBDIS;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable32 ret");
	if (TH_vmblock.enable[0] != 2) {
		printf("val: %x\n",TH_vmblock.enable[0]);
		FAIL("disable32 beh");
	}

	t0->r00 = H2K_INTOP_GLOBDIS;
	t0->r01 = PERCPU_INTERRUPTS+1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable33 ret");
	if (TH_vmblock.enable[0] != 0) FAIL("disable33 beh");

	t0->r00 = H2K_INTOP_GLOBDIS;
	t0->r01 = 1026;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("oob ret");

	puts("D");
	t0->cpuint_enabled = 0;
	TH_vmblock.enable[0] = 0;
	TH_vmblock.percpu_mask[0][0] = 0;
	t0->r00 = H2K_INTOP_LOCEN;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("enable0 ret");
	if (t0->cpuint_enabled != 0) FAIL("enable0 beh");

	t0->r00 = H2K_INTOP_LOCEN;
	t0->r01 = 1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("enable1 ret");
	if (t0->cpuint_enabled != 0) {
		printf("val: %x\n",t0->cpuint_enabled);
		FAIL("enable1 beh");
	}

	t0->r00 = H2K_INTOP_LOCEN;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable32 ret");
	if (TH_vmblock.percpu_mask[0][0] != 1) {
		printf("val: %x\n",TH_vmblock.enable[0]);
		FAIL("enable32 beh");
	}

	t0->r00 = H2K_INTOP_LOCEN;
	t0->r01 = PERCPU_INTERRUPTS+1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("enable33 ret");
	if (TH_vmblock.percpu_mask[0][0] != 3) FAIL("enable33 beh");

	t0->r00 = H2K_INTOP_LOCEN;
	t0->r01 = 1026;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("oob ret");

	puts("E");
	TH_vmblock.percpu_mask[0][0] = 3;
	t0->r00 = H2K_INTOP_LOCDIS;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("disable0 ret");
	if (t0->cpuint_enabled != 0) FAIL("disable0 beh");

	t0->r00 = H2K_INTOP_LOCDIS;
	t0->r01 = 1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("disable1 ret");
	if (t0->cpuint_enabled != 0) {
		printf("val: %x\n",t0->cpuint_enabled);
		FAIL("disable1 beh");
	}

	t0->r00 = H2K_INTOP_LOCDIS;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable32 ret");
	if (TH_vmblock.percpu_mask[0][0] != 2) {
		printf("val: %x\n",TH_vmblock.enable[0]);
		FAIL("disable32 beh");
	}

	t0->r00 = H2K_INTOP_LOCDIS;
	t0->r01 = PERCPU_INTERRUPTS+1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("disable33 ret");
	if (TH_vmblock.percpu_mask[0][0] != 0) FAIL("disable33 beh");

	t0->r00 = H2K_INTOP_LOCDIS;
	t0->r01 = 1026;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("oob ret");

	puts("F");
	t0->r00 = H2K_INTOP_AFFINITY;
	t0->r01 = 0;
	t0->r02 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("affinity ret on cpuint");

	for (i = 0; i < TH_vmblock.max_cpus; i++) {
		for (j = PERCPU_INTERRUPTS; j < TH_vmblock.num_ints; j++) {
			int maskidx = (j-PERCPU_INTERRUPTS)/32;
			int bitidx = (j-PERCPU_INTERRUPTS)%32;
			t0->r00 = H2K_INTOP_AFFINITY;
			t0->r01 = j;
			t0->r02 = i;
			H2K_vmtrap_intop(t0);
			for (k = 0; k < TH_vmblock.max_cpus; k++) {
				int bit = (TH_vmblock.percpu_mask[k][maskidx] >> bitidx) & 1;
				if (bit && (i != k)) FAIL("Enabled on non-affine CPU");
				if (!bit && (i == k)) FAIL("Disabled on affine CPU");
			}
		}
		//printf("Done: %d\n",i);
	}
	t0->r00 = H2K_INTOP_AFFINITY;
	t0->r01 = 1026;
	t0->r02 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("affinity ret on oob int");

	for (i = 0; i < TH_vmblock.max_cpus; i++) {
		for (j = PERCPU_INTERRUPTS; j < TH_vmblock.num_ints; j++) {
			int maskidx = (j-PERCPU_INTERRUPTS)/32;
			TH_vmblock.percpu_mask[i][maskidx] = 0;
		}
	}

	puts("G");
	/* GET/PEEK */
	t0->cpuint_enabled = 0;
	t0->cpuint_pending = 0;
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("peek: fail ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("get: fail ret");

	t0->cpuint_enabled = 1;
	t0->cpuint_pending = 1;
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("peek: 0 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("get: 0 ret");
	if ((t0->cpuint_enabled != 0) || (t0->cpuint_pending != 0)) FAIL("get: 0 beh");

	t0->cpuint_enabled = 3;
	t0->cpuint_pending = 3;
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("peek: 0 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("get: 0 ret");
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 1) FAIL("peek: 1 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 1) FAIL("get: 1 ret");
	if ((t0->cpuint_enabled != 0) || (t0->cpuint_pending != 0)) FAIL("get: 0 beh");

	TH_vmblock.pending[0] = 1;
	TH_vmblock.percpu_mask[0][0] = 1;
	TH_vmblock.enable[0] = 1;
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS) FAIL("peek: 32 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS) FAIL("get: 32 ret");
	if ((TH_vmblock.pending[0] != 0) || (TH_vmblock.enable[0] != 0)) FAIL("get: 32 beh");
	

	TH_vmblock.pending[0] = 3;
	TH_vmblock.percpu_mask[0][0] = 3;
	TH_vmblock.enable[0] = 3;
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS) FAIL("peek: 32 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS) FAIL("get: 32 ret");
	t0->r00 = H2K_INTOP_PEEK;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS+1) FAIL("peek: 33 ret");
	t0->r00 = H2K_INTOP_GET;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != PERCPU_INTERRUPTS+1) FAIL("get: 33 ret");
	if ((TH_vmblock.pending[0] != 0) || (TH_vmblock.enable[0] != 0)) FAIL("get: 32 beh");

	/* STATUS */
	puts("H");
	t0->cpuint_pending = 0x0000AAAA;
	t0->cpuint_enabled = 0x0000F0F0;
	for (i = 0; i < PERCPU_INTERRUPTS; i++) {
		t0->r00 = H2K_INTOP_STATUS;
		t0->r01 = i;
		H2K_vmtrap_intop(t0);
		if (t0->r00 != (i & 5)) FAIL("bad cpuint status");
	}

	TH_vmblock.pending[0] = 0xAAAAAAAA;
	TH_vmblock.percpu_mask[0][0] = 0xCCCCCCCC;
	TH_vmblock.enable[0] = 0xF0F0F0F0;
	for (i = PERCPU_INTERRUPTS; i < PERCPU_INTERRUPTS+32; i++) {
		t0->r00 = H2K_INTOP_STATUS;
		t0->r01 = i;
		H2K_vmtrap_intop(t0);
		if (t0->r00 != (i & 7)) FAIL("bad shint status");
	}

	t0->r00 = H2K_INTOP_STATUS;
	t0->r01 = 1026;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("Bad oob status");

	t0->cpuint_pending = 0;
	t0->cpuint_enabled = 0;
	TH_vmblock.pending[0] = 0;
	TH_vmblock.percpu_mask[0][0] = 0;
	TH_vmblock.enable[0] = 0;

	/* POST / CLEAR */
	puts("I");
	puts("0");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = 0;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/0");
	if (t0->cpuint_pending != 1) FAIL("Bad post beh/0");
	puts("1");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = 0;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/0/2");
	if (t0->cpuint_pending != 1) FAIL("Bad post beh/0/2");
	puts("2");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = 1;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/1");
	if (t0->cpuint_pending != 3) FAIL("Bad post beh/1");
	puts("3");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = PERCPU_INTERRUPTS;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/32");
	if (TH_vmblock.pending[0] != 1) FAIL("Bad post beh/32");
	puts("4");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = PERCPU_INTERRUPTS;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/32/2");
	if (TH_vmblock.pending[0] != 1) FAIL("Bad post beh/32/2");
	puts("5");

	t0->r00 = H2K_INTOP_POST;
	t0->r01 = PERCPU_INTERRUPTS+1;
	t0->r02 = t0->id.raw;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad post status/33");
	if (TH_vmblock.pending[0] != 3) FAIL("Bad post beh/33");
	puts("6");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/0");
	if (t0->cpuint_pending != 2) FAIL("Bad clear beh/0");
	puts("7");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = 0;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/0/2");
	if (t0->cpuint_pending != 2) FAIL("Bad clear beh/0/2");
	puts("8");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = 1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/1");
	if (t0->cpuint_pending != 0) FAIL("Bad clear beh/1");
	puts("9");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/32");
	if (TH_vmblock.pending[0] != 2) FAIL("Bad clear beh/32");
	puts("a");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = PERCPU_INTERRUPTS;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/32/2");
	if (TH_vmblock.pending[0] != 2) FAIL("Bad clear beh/32/2");
	puts("b");

	t0->r00 = H2K_INTOP_CLEAR;
	t0->r01 = PERCPU_INTERRUPTS+1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != 0) FAIL("Bad clear status/33");
	if (TH_vmblock.pending[0] != 0) FAIL("Bad clear beh/33");

	puts("J");

	t0->r00 = 99;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("OOB op");

	t0->r00 = -1;
	H2K_vmtrap_intop(t0);
	if (t0->r00 != -1) FAIL("OOB op");

	puts("TEST PASSED");
	return 0;
}

