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
#define MAX_TEST_INTERRUPTS 16

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

H2K_vmblock_t TH_vmblock;
H2K_thread_context a;

H2K_vm_int_opinfo_t info[2];

union {
	u32_t raw;
	struct {
		u32_t nop:1;
		u32_t enable:1;
		u32_t disable:1;
		u32_t localen:1;
		u32_t localdis:1;
		u32_t setaffinity:1;
		u32_t get:1;
		u32_t peek:1;
		u32_t status:1;
		u32_t post:1;
		u32_t clear:1;
	};
} TH_expected_next;

#define make_TH_next_handler(op) \
static s32_t TH_next_##op(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *nextinfo) \
{ \
	if (nextinfo->handlers != (info[1].handlers)) FAIL( #op ": Wrong info ptr"); \
	if (TH_expected_next.op == 0) FAIL( #op ": Didn't expect next nop"); \
	TH_expected_next.op = 0; \
	return -1; \
}

make_TH_next_handler(nop)
make_TH_next_handler(enable)
make_TH_next_handler(disable)
make_TH_next_handler(localen)
make_TH_next_handler(localdis)
make_TH_next_handler(setaffinity)
make_TH_next_handler(get)
make_TH_next_handler(peek)
make_TH_next_handler(status)
make_TH_next_handler(post)
make_TH_next_handler(clear)

const H2K_vm_int_ops_t H2K_vm_TH_ops = {
	.nop = TH_next_nop,
	.enable = TH_next_enable,
	.disable = TH_next_disable,
	.localen = TH_next_localen,
	.localdis = TH_next_localdis,
	.setaffinity = TH_next_setaffinity,
	.get = TH_next_get,
	.peek = TH_next_peek,
	.status = TH_next_status,
	.post = TH_next_post,
	.clear = TH_next_clear,
};

void TH_init_vmblock()
{
	memset(&TH_vmblock,0,sizeof(TH_vmblock));
	TH_vmblock.contexts = &a;
	TH_vmblock.max_cpus = 1;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	TH_vmblock.intinfo = info;
	info[0].num_ints = PERCPU_INTERRUPTS;
	info[0].handlers = &H2K_vm_cpuint_ops;
	info[1].num_ints = ~0;
	info[1].handlers = &H2K_vm_TH_ops;
	a.status = H2K_STATUS_RUNNING;
}

u32_t TH_vmstatus_setting = 0;

u32_t TH_saw_ipi_send = 0;
u32_t TH_expected_ipi = 0;

void H2K_vm_ipi_send_withlock(H2K_thread_context *thread)
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
		H2K_vm_cpuint_enable(&TH_vmblock,&a,j,info);
		if (((a.cpuint_enabled >> j) & 1) == 0) FAIL("H2K_vm_cpuint_enable");
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j,info);
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
	H2K_vm_cpuint_post(&TH_vmblock,&a,intno,info);
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
	H2K_vm_cpuint_enable(&TH_vmblock,&a,intno,info);
	TH_check_delivery(intno);
}

void TH_test_clear(u32_t intno)
{
	a.cpuint_pending |= (1<<(intno%16));
	H2K_vm_cpuint_clear(&TH_vmblock,&a,intno,info);
	if ((a.cpuint_pending >> (intno)) & 1) FAIL("Didn't clear interrupt");
}

void TH_test_posting()
{
	u32_t j;
	/* Test that a normally posted interrupt will cause a thread to have VM Work */
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j,info);
		TH_test_post(j);
		a.cpuint_pending = 0;
		H2K_vm_cpuint_enable(&TH_vmblock,&a,j,info);
		TH_test_post(j);
	}
	/* Test that a disabled interrupt that is pending and gets enabled will cause VM Work */
	/* Note that for cpu interrupt, we don't need to IPI because we always enable ourselves */
	TH_expected_ipi = 0;
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		H2K_vm_cpuint_disable(&TH_vmblock,&a,j,info);
		TH_check_enable_wakeup(j);
	}
	/* Test that a posted interrupt can be cleared */
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		TH_test_clear(j);
	}
}

void TH_test_nexts()
{
	H2K_thread_context *me = &a;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS+1;
#define TEST_NEXT(OP) \
	TH_expected_next.OP = 1; \
	H2K_vm_cpuint_ops.OP(&TH_vmblock,me,MAX_TEST_INTERRUPTS,info); \
	if (TH_expected_next.raw != 0) FAIL("Didn't get next " #OP);

	TEST_NEXT(enable)
	TEST_NEXT(disable)
	TEST_NEXT(localen)
	TEST_NEXT(localdis)
	TEST_NEXT(setaffinity)
	TEST_NEXT(status)
	TEST_NEXT(post)
	TEST_NEXT(clear)
}

void TH_test_interrupt_get(u32_t intno)
{
	u32_t ret;
	/* Starts with globally disabled / not pending */
	H2K_thread_context *me = &a;
	me->vmstatus = 0;
	/* Disabled globally / not pending */
	TH_expected_next.get = 1;
	if (H2K_vm_cpuint_get(&TH_vmblock,me,0,info) != -1) FAIL("Got disabled/np int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next/a");
	/* Enabled globally / not pending */
	H2K_vm_cpuint_enable(&TH_vmblock,me,intno,info);
	TH_expected_next.get = 1;
	if (H2K_vm_cpuint_get(&TH_vmblock,me,0,info) != -1) FAIL("Got np int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next/b");
	/* Disabled globally / pending */
	H2K_vm_cpuint_disable(&TH_vmblock,me,intno,info);
	a.cpuint_pending |= (1<<(intno));
	TH_expected_next.get = 1;
	if (H2K_vm_cpuint_get(&TH_vmblock,me,0,info) != -1) FAIL("Got disabled int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next/c");
	/* Enabled globally / pending */
	a.cpuint_enabled |= (1<<(intno));
	if ((ret = H2K_vm_cpuint_get(&TH_vmblock,me,0,info)) != intno) {
		//printf("enabed=%x pending=%x ret=%x expected=%x\n",a.cpuint_enabled,a.cpuint_pending,ret,intno);
		FAIL("Didn't get expected interrupt");
	}
	if ((a.cpuint_enabled & (1<<(intno))) != 0) {
		FAIL("Not auto-disabled");
	}
	if ((a.cpuint_pending & (1<<(intno))) != 0) {
		FAIL("Not cleared from pending");
	}
	H2K_vm_cpuint_disable(&TH_vmblock,me,intno,info);
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
		tmp = H2K_vm_cpuint_status(&TH_vmblock,&a,i,info);
		if (tmp != (i & 0x5)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7)));
			FAIL("Bad bits");
		}
	}
}

int main()
{
	int j;
	puts("0");
	TH_init_vmblock();

	/* Check status */
	puts("1");
	TH_test_status();

	puts("2");
	/* int_v2p not set up */
	/* pmap not set up */
	/* Set set/clear mask functions basic functionality */
	TH_test_maskfuncs();

	puts("3");
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

	/* Test next functions */
	TH_test_nexts();
	puts("d");

	/* Further get testing */
	a.cpuint_pending = 0x0000ffff;
	a.cpuint_enabled = 0x0000ffff;
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a,0,info) != 0) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	if (H2K_vm_cpuint_get(&TH_vmblock,&a,0,info) != 0) {
		FAIL("Didn't get first valid interrupt/get");
	}
	if (a.cpuint_pending != 0x0000fffe) {
		FAIL("Didn't clear pending bit");
	}
	if (a.cpuint_enabled != 0x0000fffe) {
		FAIL("Didn't clear enable bit");
	}
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a,0,info) != 1) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	a.cpuint_pending = 0;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	TH_expected_next.peek = 1;
	if (H2K_vm_cpuint_peek(&TH_vmblock,&a,0,info) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (TH_expected_next.raw != 0) FAIL("didn't get next");
	TH_expected_next.get = 1;
	if (H2K_vm_cpuint_get(&TH_vmblock,&a,0,info) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (TH_expected_next.raw != 0) FAIL("didn't get next");
	/* OK!  We're done here! */
	puts("TEST PASSED");
	return 0;
}

