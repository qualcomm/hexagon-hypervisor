/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * There are several functions to test:
 * H2K_vm_shint_post
 * H2K_vm_shint_disable
 * H2K_vm_shint_enable
 * H2K_vm_shint_localen
 * H2K_vm_shint_localdis
 * H2K_vm_shint_setaffinity
 * H2K_vm_shint_get
 * 
 * Here is the plan:
 * + Iterate over various numbers of total interrupts
 * + Iterate over various numbers of CPUs
 * + Iterate over CPUs interrupt enable/disabled
 * + Set up vmblock correctly
 * + Enable/Disable value checks
 * + Localmask/Localunmask value checks
 * + Setaffinity mask value checks
 * + Enable/Disable, then Post, check values
 * + Localmask/Localunmask, then Post, check values
 * + Enable/Disable, then Get, check values
 * + Localmask/Localunmask, then Get, check values
 * + Enable / check for delivery
 * + Localunmask / check for delivery
 * 
 */

#define MAX_TEST_THREADS 8
#define MAX_TEST_INTERRUPTS 512

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <vm.h>
#include <context.h>
#include <string.h>
#include <shint.h>
#include <max.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context TH_threads[MAX_TEST_THREADS];
u32_t TH_pending_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_enable_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_localdis_storage[MAX_TEST_THREADS][MAX_TEST_INTERRUPTS/32];
u32_t *TH_localdis_ptr_storage[MAX_TEST_THREADS];

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
	if (TH_expected_next.op == 0) FAIL( #op ": Didn't expect next " #op); \
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

H2K_vmblock_t TH_vmblock;

void TH_init_vmblock()
{
	int i,j;
	memset(&TH_vmblock,0,sizeof(TH_vmblock));
	TH_vmblock.contexts = &TH_threads[0];
	TH_vmblock.max_cpus = MAX_TEST_THREADS;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	TH_vmblock.pending = &TH_pending_storage[0];
	TH_vmblock.enable = &TH_enable_storage[0];
	TH_vmblock.percpu_mask = &TH_localdis_ptr_storage[0];
	for (i = 0; i < MAX_TEST_INTERRUPTS/32; i++) {
		TH_pending_storage[i] = 0;
		TH_enable_storage[i] = 0;
	}
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		memset(&TH_threads[i],0,sizeof(TH_threads[i]));
		TH_threads[i].id.cpuidx = i;
		TH_threads[i].status = H2K_STATUS_RUNNING;
		TH_localdis_ptr_storage[i] = &TH_localdis_storage[i][0];
		for (j = 0; j < MAX_TEST_INTERRUPTS/32; j++) {
			TH_localdis_storage[i][j] = 0;
		}
	}
	info[0].num_ints = MAX_INTERRUPTS;
	info[0].handlers = &H2K_vm_shint_ops;
	info[1].num_ints = ~0;
	info[1].handlers = &H2K_vm_TH_ops;
}

u8_t primes[MAX_TEST_THREADS] = { 2, 3, 5, 7, 11, 13, 17, 19 };

u32_t TH_vmstatus_setting = 0;

u32_t TH_saw_ipi_send = 0;
u32_t TH_expected_ipi = 0;

void H2K_vm_ipi_send_withlock(H2K_thread_context *thread)
{
	TH_saw_ipi_send = 1;
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
			TH_localdis_storage[i][j/32] |= (1<<(j % 32));
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
			TH_localdis_storage[i][j/32] = 0;
		}
	}
}

void TH_test_affinity(u32_t j)
{
	u32_t i,k;
	TH_vmblock.max_cpus = MAX_TEST_THREADS;
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		/* Start out with all interrupts unmasked, just for fun */
		H2K_vm_shint_localen(&TH_vmblock,&TH_threads[i],j,info);
	}
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		H2K_vm_shint_setaffinity(&TH_vmblock,&TH_threads[i],j,info);
		for (k = 0; k < MAX_TEST_THREADS; k++) {
			if (i == k) {
				if ((TH_localdis_storage[k][j/32] & (1<<(j%32))) == 0) {
					FAIL("Setaffinity did not enable for thread");
				} 
			} else {
				if ((TH_localdis_storage[k][j/32] & (1<<(j%32))) != 0) {
					FAIL("Setaffinity did not disable for thread");
				} 
			}
		}
	}
}

void TH_test_maskfuncs()
{
	u32_t i,j;
	TH_clear_data();
	for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
		/* Test enable/disable */
		if ((TH_enable_storage[j/32] & (1<<(j % 32))) != 0) {
			FAIL("Interrupt not cleared correctly");
		}
		if(j == MAX_TEST_INTERRUPTS/2) {
			H2K_vm_shint_nop(&TH_vmblock,&TH_threads[0],j,info);
		}
		H2K_vm_shint_enable(&TH_vmblock,&TH_threads[0],j,info);
		if ((TH_enable_storage[j/32] & (1<<(j % 32))) == 0) {
			FAIL("H2K_vm_shint_enable failed");
		}
		for (i = 0; i < MAX_TEST_THREADS; i++) {
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("Localmask not cleared correctly");
			}
			H2K_vm_shint_localen(&TH_vmblock,&TH_threads[i],j,info);
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) == 0) {
				FAIL("localen did not set local enable");
			}
			H2K_vm_shint_localdis(&TH_vmblock,&TH_threads[i],j,info);
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("localdis did not clear local enable");
			}
		}
		H2K_vm_shint_disable(&TH_vmblock,&TH_threads[i],j,info);
		for (i = 0; i < MAX_TEST_THREADS; i++) {
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("Localmask not cleared correctly");
			}
			H2K_vm_shint_localen(&TH_vmblock,&TH_threads[i],j,info);
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) == 0) {
				FAIL("localen did not set local enable");
			}
			H2K_vm_shint_localdis(&TH_vmblock,&TH_threads[i],j,info);
			if ((TH_localdis_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("localdis did not clear local enable");
			}
		}
		if ((TH_enable_storage[j/32] & (1<<(j % 32))) != 0) {
			FAIL("H2K_vm_shint_disable failed");
		}
		/* Test setaffinity */
		TH_test_affinity(j);
	}
}

void TH_check_delivery(u32_t cpu, u32_t intno)
{
	u32_t k;
	u32_t saw_post = 0;
	k = cpu;
	do {
		if ((((TH_vmblock.enable[intno/32]) & 
		     (TH_vmblock.percpu_mask[k][intno/32]) &
		     (1<<(intno%32))) != 0)) {
			if (saw_post && 
			    (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK)) {
				FAIL("Saw post and additional vmwork set");
			} else if (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK) {
				TH_check_ipi();
				saw_post = 1;
			} else if (!saw_post) {
				printf("Pending=%x,Enable=%x,mask=%x,intno=%d,th=%d/%d,cpu=%d\n",
					TH_vmblock.pending[intno/32],
					TH_vmblock.enable[intno/32],
					TH_vmblock.percpu_mask[k][intno/32],
					intno,k,TH_vmblock.max_cpus,cpu);
				for (k = 0; k < TH_vmblock.max_cpus; k++) {
					printf("TH %d: %x (status=%x)\n",k,TH_threads[k].vmstatus,TH_threads[k].status);
				}
				FAIL("DId not set first thread");
			}
		} else if (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK) {
			FAIL("Put work on wrong thread");
		}
		if (++k >= TH_vmblock.max_cpus) k = 0;
	} while (k != cpu);
}

void TH_test_post(u32_t cpu, u32_t intno)
{
	u32_t k;
	for (k = 0; k < MAX_TEST_THREADS; k++) {
		TH_threads[k].vmstatus = TH_vmstatus_setting;
	}
	//printf("Posting cpu=%d intno=%d\n",cpu,intno);
	H2K_vm_shint_post(&TH_vmblock,&TH_threads[cpu], intno,info);
	if ((TH_vmblock.pending[intno/32] & (1<<(intno%32))) == 0) {
		FAIL("Did not pend interrupt");
	}
	TH_check_delivery(cpu,intno);
}

void TH_check_enable_wakeup(u32_t intno)
{
	u32_t k;
	for (k = 0; k < MAX_TEST_THREADS; k++) {
		TH_threads[k].vmstatus = TH_vmstatus_setting;
	}
	TH_vmblock.pending[intno/32] = 0xffffffff;
	H2K_vm_shint_enable(&TH_vmblock,&TH_threads[0],intno,info);
	TH_check_delivery(0,intno);
}

void TH_check_localen_wakeup(u32_t cpu, u32_t intno)
{
	u32_t k;
	for (k = 0; k < MAX_TEST_THREADS; k++) {
		TH_threads[k].vmstatus = TH_vmstatus_setting;
	}
	TH_vmblock.pending[intno/32] = 0xffffffff;
	H2K_vm_shint_localen(&TH_vmblock,&TH_threads[cpu],intno,info);
	TH_check_delivery(cpu,intno);
	if ((TH_threads[cpu].vmstatus & H2K_VMSTATUS_IE) && 
		((TH_threads[cpu].vmstatus & H2K_VMSTATUS_VMWORK)== 0)) {
		FAIL("Didn't interrupt unmasking thread");
	}
}

void TH_test_clear(u32_t intno)
{
	TH_vmblock.pending[intno/32] |= (1<<(intno%32));
	H2K_vm_shint_clear(&TH_vmblock,&TH_threads[0],intno,info);
	if ((TH_vmblock.pending[intno/32] >> (intno % 32)) & 1) FAIL("Didn't clear interrupt");
}

void TH_test_posting()
{
	u32_t i,j;
	/* Test that a normally posted interrupt will cause a thread to have VM Work */
	puts("W");
	for (j = 0; j < TH_vmblock.num_ints; j++) {
		for (i = 0; i < TH_vmblock.max_cpus; i++) {
			//printf("i,j=%d,%d\n",i,j);
			H2K_vm_shint_disable(&TH_vmblock,&TH_threads[0],j,info);
			TH_test_post(i,j);
			//printf("Disable OK\n");
			TH_vmblock.pending[j/32] = 0;
			H2K_vm_shint_enable(&TH_vmblock,&TH_threads[0],j,info);
			TH_test_post(i,j);
			//printf("Enable OK\n");
		}
	}
	/* Test that a disabled interrupt that is pending and gets enabled will cause VM Work */
	puts("X");
	for (j = 0; j < TH_vmblock.num_ints; j++) {
		H2K_vm_shint_disable(&TH_vmblock,&TH_threads[0],j,info);
		TH_check_enable_wakeup(j);
		for (i = 0; i < TH_vmblock.max_cpus; i++) {
			if ((TH_vmblock.percpu_mask[i][j/32] & (1<<(j%32))) != 0) {
				H2K_vm_shint_localdis(&TH_vmblock,&TH_threads[i],j,info);
				TH_check_localen_wakeup(i,j);
			}
		}
	}
	/* Test that a posted interrupt can be cleared */
	puts("Y");
	for (j = 0; j < TH_vmblock.num_ints; j++) {
		TH_test_clear(j);
	}
}

void TH_test_interrupt_get(u32_t cpu,u32_t intno)
{
	/* Starts with globally disabled / not pending */
	H2K_thread_context *me = &TH_threads[cpu];
	me->vmstatus = 0;
	/* Disabled globally / not pending */
	TH_expected_next.get = 1;
	if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[cpu],0,info) != -1) FAIL("Got disabled/np int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next");
	/* Enabled globally / not pending */
	H2K_vm_shint_enable(&TH_vmblock,&TH_threads[0],intno,info);
	TH_expected_next.get = 1;
	if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[cpu],0,info) != -1) FAIL("Got np int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next");
	/* Disabled globally / pending */
	H2K_vm_shint_disable(&TH_vmblock,&TH_threads[0],intno,info);
	TH_vmblock.pending[intno/32] |= (1<<(intno%32));
	TH_expected_next.get = 1;
	if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[cpu],0,info) != -1) FAIL("Got disabled int");
	if (TH_expected_next.raw != 0) FAIL("didn't get next");
	/* Enabled globally / pending */
	TH_vmblock.enable[intno/32] |= (1<<(intno%32));
	if ((TH_vmblock.percpu_mask[cpu][intno/32] & (1<<(intno%32))) != 0) {
		if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[cpu],0,info) != intno) {
			FAIL("Didn't get expected interrupt");
		}
		if ((TH_vmblock.enable[intno/32] & (1<<(intno%32))) != 0) {
			FAIL("Not auto-disabled");
		}
		if ((TH_vmblock.pending[intno/32] & (1<<(intno%32))) != 0) {
			FAIL("Not cleared from pending");
		}
	} else {
		TH_expected_next.get = 1;
		if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[cpu],0,info) != -1) {
			FAIL("Didn't get expected interrupt");
		}
		if (TH_expected_next.raw != 0) FAIL("didn't get next");
	}
	H2K_vm_shint_disable(&TH_vmblock,&TH_threads[0],intno,info);
	TH_vmblock.pending[intno/32] = 0;
}

void TH_test_status()
{
	int i;
	u32_t tmp;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	for (i = 0; i < TH_vmblock.num_ints/32; i++) {
		TH_vmblock.enable[i] = 0;
		TH_vmblock.percpu_mask[0][i] = 0;
		TH_vmblock.percpu_mask[1][i] = 0;
		TH_vmblock.pending[i] = 0;
	}
	for (i = 0; i < TH_vmblock.num_ints; i++) {
		if (i & 4) TH_vmblock.enable[i/32] |= (1<<(i%32));
		TH_vmblock.percpu_mask[(i & 2) == 0][i/32] |= (1<<(i%32));
		if (i & 1) TH_vmblock.pending[i/32] |= (1<<(i%32));
	}
	for (i = 0; i < TH_vmblock.num_ints; i++) {
		tmp = H2K_vm_shint_status(&TH_vmblock,&TH_threads[0],i,info);
		if (tmp != (i & 0x7)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7)));
			FAIL("Bad bits/0");
		}
		tmp = H2K_vm_shint_status(&TH_vmblock,&TH_threads[1],i,info);
		if (tmp != ((i & 0x7) ^ 2)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7) ^ 2));
			FAIL("Bad bits/1");
		}
	}
}

void TH_test_nexts()
{
	H2K_thread_context *me = &TH_threads[0];
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS+1;
#define TEST_NEXT(OP) \
	TH_expected_next.OP = 1; \
	H2K_vm_shint_ops.OP(&TH_vmblock,me,MAX_TEST_INTERRUPTS,info); \
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

int main()
{
	int i,j;
	TH_init_vmblock();

	/* Check status */
	TH_test_status();

	/* int_v2p not set up */
	/* pmap not set up */
	/* Set set/clear mask functions basic functionality */
	TH_test_maskfuncs();

	/* Set up post/get test masks */
	TH_setup_intmask();

	/* Set up for interrupt disabled checks */
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	TH_vmstatus_setting = 0;
	TH_expected_ipi = 0;
	puts("A");
	for (i = 1; i < MAX_TEST_THREADS; i++) {
		TH_vmblock.max_cpus = i;
		printf("Num_cpus=%d\n",i);
		TH_test_posting();
	}
	puts("B");

	/* Set up for interrupt enabled checks */
	TH_vmstatus_setting = H2K_VMSTATUS_IE;
	TH_expected_ipi = 1;
	for (i = 1; i < MAX_TEST_THREADS; i++) {
		TH_vmblock.max_cpus = i;
		printf("Num_cpus=%d\n",i);
		TH_test_posting();
	}
	puts("C");

	/* Test interrupt get */
	TH_vmblock.max_cpus = MAX_TEST_THREADS;
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;
	for (j = 0; j < MAX_TEST_INTERRUPTS; j+=32) {
		TH_vmblock.pending[j/32] = 0;
		TH_vmblock.enable[j/32] = 0;
	}
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		for (j = 0; j < MAX_TEST_INTERRUPTS; j++) {
			TH_test_interrupt_get(i,j);
		}
	}

	puts("D");
	TH_test_nexts();

	puts("E");
	/* Further get testing */
	TH_vmblock.pending[0] = 0xffffffff;
	TH_vmblock.enable[0] = 0xffffffff;
	TH_vmblock.percpu_mask[0][0] = 0xffffffff;
	if (H2K_vm_shint_peek(&TH_vmblock,&TH_threads[0],0,info) != 0) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[0],0,info) != 0) {
		FAIL("Didn't get first valid interrupt/get");
	}
	if (TH_vmblock.pending[0] != 0xfffffffe) {
		FAIL("Didn't clear pending bit");
	}
	if (TH_vmblock.enable[0] != 0xfffffffe) {
		FAIL("Didn't clear enable bit");
	}
	if (H2K_vm_shint_peek(&TH_vmblock,&TH_threads[0],0,info) != 1) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	TH_vmblock.pending[0] = 0;
	TH_vmblock.num_ints = 32;
	TH_expected_next.peek = 1;
	if (H2K_vm_shint_peek(&TH_vmblock,&TH_threads[0],0,info) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (TH_expected_next.raw != 0) FAIL("didn't get next/a");
	TH_expected_next.get = 1;
	if (H2K_vm_shint_get(&TH_vmblock,&TH_threads[0],0,info) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (TH_expected_next.raw != 0) FAIL("didn't get next/a");
	/* OK!  We're done here! */
	puts("TEST PASSED");
	return 0;
}

