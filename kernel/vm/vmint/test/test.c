/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * There are several functions to test:
 * H2K_vm_interrupt_post
 * H2K_vm_interrupt_disable
 * H2K_vm_interrupt_enable
 * H2K_vm_interrupt_localunmask
 * H2K_vm_interrupt_localmask
 * H2K_vm_interrupt_setaffinity
 * H2K_vm_interrupt_get
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
#include <vmint.h>
#include <vm.h>
#include <context.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context TH_threads[MAX_TEST_THREADS];
H2K_thread_context *TH_thread_ptrs[MAX_TEST_THREADS];
u32_t TH_pending_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_enable_storage[MAX_TEST_INTERRUPTS/32];
u32_t TH_localmask_storage[MAX_TEST_THREADS][MAX_TEST_INTERRUPTS/32];
u32_t *TH_localmask_ptr_storage[MAX_TEST_THREADS];

H2K_vmblock_t TH_vmblock;

u8_t primes[MAX_TEST_THREADS] = { 2, 3, 5, 7, 11, 13, 17, 19 };

u32_t TH_vmstatus_setting = 0;

u32_t TH_saw_ipi_send = 0;
u32_t TH_expected_ipi = 0;

void H2K_vm_ipi_send(H2K_thread_context *thread)
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

void TH_test_affinity(u32_t j)
{
	u32_t i,k;
	TH_vmblock.num_cpus = MAX_TEST_THREADS;
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		/* Start out with all interrupts unmasked, just for fun */
		H2K_vm_interrupt_localunmask(&TH_vmblock,i,j);
	}
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		H2K_vm_interrupt_setaffinity(&TH_vmblock,i,j);
		for (k = 0; k < MAX_TEST_THREADS; k++) {
			if (i == k) {
				if ((TH_localmask_storage[k][j/32] & (1<<(j%32))) == 0) {
					FAIL("Setaffinity did not enable for thread");
				} 
			} else {
				if ((TH_localmask_storage[k][j/32] & (1<<(j%32))) != 0) {
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
		H2K_vm_interrupt_enable(&TH_vmblock,j);
		if ((TH_enable_storage[j/32] & (1<<(j % 32))) == 0) {
			FAIL("H2K_vm_interrupt_enable failed");
		}
		for (i = 0; i < MAX_TEST_THREADS; i++) {
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("Localmask not cleared correctly");
			}
			H2K_vm_interrupt_localunmask(&TH_vmblock,i,j);
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) == 0) {
				FAIL("localunmask did not set local enable");
			}
			H2K_vm_interrupt_localmask(&TH_vmblock,i,j);
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("localmask did not clear local enable");
			}
		}
		H2K_vm_interrupt_disable(&TH_vmblock,j);
		for (i = 0; i < MAX_TEST_THREADS; i++) {
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("Localmask not cleared correctly");
			}
			H2K_vm_interrupt_localunmask(&TH_vmblock,i,j);
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) == 0) {
				FAIL("localunmask did not set local enable");
			}
			H2K_vm_interrupt_localmask(&TH_vmblock,i,j);
			if ((TH_localmask_storage[i][j/32] & (1<<(j%32))) != 0) {
				FAIL("localmask did not clear local enable");
			}
		}
		if ((TH_enable_storage[j/32] & (1<<(j % 32))) != 0) {
			FAIL("H2K_vm_interrupt_disable failed");
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
		if (((TH_vmblock.enable[intno/32]) & 
		     (TH_vmblock.percpu_mask[k][intno/32]) &
		     (1<<(intno%32))) != 0) {
			if (saw_post && 
			    (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK)) {
				FAIL("Saw post and additional vmwork set");
			} else if (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK) {
				TH_check_ipi();
				saw_post = 1;
			} else if (!saw_post) {
				printf("Enable=%x,mask=%x,intno=%d,th=%d/%d,cpu=%d\n",
					TH_vmblock.enable[intno/32],
					TH_vmblock.percpu_mask[k][intno/32],
					intno,k,TH_vmblock.num_cpus,cpu);
				for (k = 0; k < TH_vmblock.num_cpus; k++) {
					printf("TH %d: %d\n",k,TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK);
				}
				FAIL("DId not set first thread");
			}
		} else if (TH_threads[k].vmstatus & H2K_VMSTATUS_VMWORK) {
			FAIL("Put work on wrong thread");
		}
		if (++k >= TH_vmblock.num_cpus) k = 0;
	} while (k != cpu);
}

void TH_test_post(u32_t cpu, u32_t intno)
{
	u32_t k;
	for (k = 0; k < MAX_TEST_THREADS; k++) {
		TH_threads[k].vmstatus = TH_vmstatus_setting;
	}
	H2K_vm_interrupt_post(&TH_vmblock,cpu, intno);
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
	H2K_vm_interrupt_enable(&TH_vmblock,intno);
	TH_check_delivery(0,intno);
}

void TH_check_localunmask_wakeup(u32_t cpu, u32_t intno)
{
	u32_t k;
	for (k = 0; k < MAX_TEST_THREADS; k++) {
		TH_threads[k].vmstatus = TH_vmstatus_setting;
	}
	TH_vmblock.pending[intno/32] = 0xffffffff;
	H2K_vm_interrupt_localunmask(&TH_vmblock,cpu,intno);
	TH_check_delivery(cpu,intno);
	if (TH_threads[cpu].vmstatus == 0) FAIL("Didn't interrupt unmasking thread");
}

void TH_test_clear(u32_t intno)
{
	TH_vmblock.pending[intno/32] |= (1<<(intno%32));
	H2K_vm_interrupt_clear(&TH_vmblock,intno);
	if ((TH_vmblock.pending[intno/32] >> (intno % 32)) & 1) FAIL("Didn't clear interrupt");
}

void TH_test_posting()
{
	u32_t i,j;
	/* Test that a normally posted interrupt will cause a thread to have VM Work */
	for (j = 0; j < TH_vmblock.num_ints; j++) {
		for (i = 0; i < TH_vmblock.num_cpus; i++) {
			H2K_vm_interrupt_disable(&TH_vmblock,j);
			TH_test_post(i,j);
			TH_vmblock.pending[j/32] = 0;
			H2K_vm_interrupt_enable(&TH_vmblock,j);
			TH_test_post(i,j);
		}
	}
	/* Test that a disabled interrupt that is pending and gets enabled will cause VM Work */
	for (j = 0; j < TH_vmblock.num_ints; j++) {
		H2K_vm_interrupt_disable(&TH_vmblock,j);
		TH_check_enable_wakeup(j);
		for (i = 0; i < TH_vmblock.num_cpus; i++) {
			if ((TH_vmblock.percpu_mask[i][j/32] & (1<<(j%32))) != 0) {
				H2K_vm_interrupt_localmask(&TH_vmblock,i,j);
				TH_check_localunmask_wakeup(i,j);
			}
		}
	}
	/* Test that a posted interrupt can be cleared */
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
	if (H2K_vm_interrupt_get(&TH_vmblock,cpu) != -1) FAIL("Got disabled/np int");
	/* Enabled globally / not pending */
	H2K_vm_interrupt_enable(&TH_vmblock,intno);
	if (H2K_vm_interrupt_get(&TH_vmblock,cpu) != -1) FAIL("Got np int");
	/* Disabled globally / pending */
	H2K_vm_interrupt_disable(&TH_vmblock,intno);
	TH_vmblock.pending[intno/32] |= (1<<(intno%32));
	if (H2K_vm_interrupt_get(&TH_vmblock,cpu) != -1) FAIL("Got disabled int");
	/* Enabled globally / pending */
	TH_vmblock.enable[intno/32] |= (1<<(intno%32));
	if ((TH_vmblock.percpu_mask[cpu][intno/32] & (1<<(intno%32))) != 0) {
		if (H2K_vm_interrupt_get(&TH_vmblock,cpu) != intno) {
			FAIL("Didn't get expected interrupt");
		}
		if ((TH_vmblock.enable[intno/32] & (1<<(intno%32))) != 0) {
			FAIL("Not auto-disabled");
		}
		if ((TH_vmblock.pending[intno/32] & (1<<(intno%32))) != 0) {
			FAIL("Not cleared from pending");
		}
	} else {
		if (H2K_vm_interrupt_get(&TH_vmblock,cpu) != -1) {
			FAIL("Didn't get expected interrupt");
		}
	}
	H2K_vm_interrupt_disable(&TH_vmblock,intno);
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
		if (i & 2) TH_vmblock.percpu_mask[0][i/32] |= (1<<(i%32));
		else       TH_vmblock.percpu_mask[1][i/32] |= (1<<(i%32));
		if (i & 1) TH_vmblock.pending[i/32] |= (1<<(i%32));
	}
	for (i = 0; i < TH_vmblock.num_ints; i++) {
		tmp = H2K_vm_interrupt_status(&TH_vmblock,0,i);
		if (tmp != (i & 0x7)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7)));
			FAIL("Bad bits/0");
		}
		tmp = H2K_vm_interrupt_status(&TH_vmblock,1,i);
		if (tmp != ((i & 0x7) ^ 2)) {
			printf("ret: %x i: %x expect: %x\n",tmp,i,((i & 7) ^ 2));
			FAIL("Bad bits/1");
		}
	}
}

int main()
{
	int i,j;
	TH_vmblock.pending = TH_pending_storage;
	TH_vmblock.enable = TH_enable_storage;
	TH_vmblock.percpu_mask = TH_localmask_ptr_storage;
	TH_vmblock.cpu_contexts = TH_thread_ptrs;
	for (i = 0; i < MAX_TEST_THREADS; i++) {
		TH_thread_ptrs[i] = &TH_threads[i];
		TH_vmblock.percpu_mask[i] = TH_localmask_storage[i];
	}
	TH_vmblock.num_ints = MAX_TEST_INTERRUPTS;

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
	for (i = 1; i < MAX_TEST_THREADS; i++) {
		TH_vmblock.num_cpus = i;
		TH_test_posting();
	}

	/* Set up for interrupt enabled checks */
	TH_vmstatus_setting = H2K_VMSTATUS_IE;
	TH_expected_ipi = 1;
	for (i = 1; i < MAX_TEST_THREADS; i++) {
		TH_vmblock.num_cpus = i;
		TH_test_posting();
	}

	/* Test interrupt get */
	TH_vmblock.num_cpus = MAX_TEST_THREADS;
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
	/* Further get testing */
	TH_vmblock.pending[0] = 0xffffffff;
	TH_vmblock.enable[0] = 0xffffffff;
	TH_vmblock.percpu_mask[0][0] = 0xffffffff;
	if (H2K_vm_interrupt_peek(&TH_vmblock,0) != 0) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	if (H2K_vm_interrupt_get(&TH_vmblock,0) != 0) {
		FAIL("Didn't get first valid interrupt/get");
	}
	if (TH_vmblock.pending[0] != 0xfffffffe) {
		FAIL("Didn't clear pending bit");
	}
	if (TH_vmblock.enable[0] != 0xfffffffe) {
		FAIL("Didn't clear enable bit");
	}
	if (H2K_vm_interrupt_peek(&TH_vmblock,0) != 1) {
		FAIL("Didn't get first valid interrupt/peek");
	}
	TH_vmblock.pending[0] = 0;
	TH_vmblock.num_ints = 32;
	if (H2K_vm_interrupt_peek(&TH_vmblock,0) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	if (H2K_vm_interrupt_get(&TH_vmblock,0) != -1) {
		FAIL("Found interrupt, but shouldn't have");
	}
	/* OK!  We're done here! */
	puts("TEST PASSED");
	return 0;
}

