/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

/*
 * This test checks the following functionality:
 * Futex wait on invalid locks
 * Futex resume on invalid locks
 * PI Mutex Lock on Invalid Locks
 * PI Mutex Unlock on Invalid Locks
 */

#define BADBASE 0xa0000000
#define PERMS(X) (0x90000000 | ((X) << 24))

h2_sem_t sema;
h2_sem_t semb;

void FAIL(const char *str)
{
	printf("FAIL: %s\n",str);
	exit(1);
}

void test_ptr_all(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success");
	if (h2_futex_wake(ptr,1) >= 0) FAIL("Wake returned success");
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_w(void *ptr)
{
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_rok(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success?");
	if (h2_futex_wake(ptr,1) > 0) FAIL("Wake woke thread?");
}

unsigned long long int context_space[512];

#define THREAD_STACK_SIZE 512
#define NUM_TOTAL_THREADS 5

unsigned long long int stack_space[THREAD_STACK_SIZE];
unsigned long long int main_thread_stack[THREAD_STACK_SIZE];

void worker_thread(void *param)
{
	h2_sem_t *my_sema = (void *)(PERMS(7) | (u32_t)(&sema));
	h2_sem_t *my_semb = (void *)(PERMS(7) | (u32_t)(&semb));
	printf("worker thread: my_sema=%p my_semb=%p\n",my_sema,my_semb);
	h2_sem_up(my_sema);
	h2_sem_down(my_semb);
	h2_sem_up(my_sema);
}

#define TRANS_ENTRY(VPN,PPN,SIZE,CCCC,PERMS) \
	( (((unsigned long long int)((SIZE) & 0xF)) << (32+20)) \
	| (((unsigned long long int)((VPN) & 0xFFFFF)) << (32)) \
	| (((unsigned long long int)((PERMS) & 0xF)) << (28)) \
	| (((unsigned long long int)((CCCC) & 0xF)) << (24)) \
	| (((unsigned long long int)((PPN) & 0xFFFFFF)) << (0)))

#define H2K_GUEST_START_PAGE (H2K_GUEST_START >> 12)

unsigned long long int transtab[] = {
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x00000 ,H2K_GUEST_START_PAGE + 0x00000,6,0x7,0xf),
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x01000 ,H2K_GUEST_START_PAGE + 0x01000,6,0x7,0xf),
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x02000 ,H2K_GUEST_START_PAGE + 0x02000,6,0x7,0xf),
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x03000 ,H2K_GUEST_START_PAGE + 0x03000,6,0x7,0xf),
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x04000 ,H2K_GUEST_START_PAGE + 0x04000,6,0x7,0xf),
	TRANS_ENTRY(H2K_GUEST_START_PAGE + 0x05000 ,H2K_GUEST_START_PAGE + 0x05000,6,0x7,0xf),

	TRANS_ENTRY(0x90000,H2K_GUEST_START_PAGE,6,0x7,0x0),
	TRANS_ENTRY(0x91000,H2K_GUEST_START_PAGE,6,0x7,0x1),
	TRANS_ENTRY(0x92000,H2K_GUEST_START_PAGE,6,0x7,0x2),
	TRANS_ENTRY(0x93000,H2K_GUEST_START_PAGE,6,0x7,0x3),
	TRANS_ENTRY(0x94000,H2K_GUEST_START_PAGE,6,0x7,0x4),
	TRANS_ENTRY(0x95000,H2K_GUEST_START_PAGE,6,0x7,0x5),
	TRANS_ENTRY(0x96000,H2K_GUEST_START_PAGE,6,0x7,0x6),
	TRANS_ENTRY(0x97000,H2K_GUEST_START_PAGE,6,0x7,0x7),
	TRANS_ENTRY(0x98000,H2K_GUEST_START_PAGE,6,0x7,0x8),
	TRANS_ENTRY(0x99000,H2K_GUEST_START_PAGE,6,0x7,0x9),
	TRANS_ENTRY(0x9a000,H2K_GUEST_START_PAGE,6,0x7,0xa),
	TRANS_ENTRY(0x9b000,H2K_GUEST_START_PAGE,6,0x7,0xb),
	TRANS_ENTRY(0x9c000,H2K_GUEST_START_PAGE,6,0x7,0xc),
	TRANS_ENTRY(0x9d000,H2K_GUEST_START_PAGE,6,0x7,0xd),
	TRANS_ENTRY(0x9e000,H2K_GUEST_START_PAGE,6,0x7,0xe),
	TRANS_ENTRY(0x9f000,H2K_GUEST_START_PAGE,6,0x7,0xf),

	0,
};

/* FIXME: looks like this test is unfinished; it doesn't do much (?) */
int main(int argc, char **argv)
{
	int i;
	printf("VM Main\n");
	h2_handle_errors(1);
	h2_vmtrap_newmap(transtab,H2K_ASID_TRANS_TYPE_LINEAR,0);
	puts("mapped\n");
	h2_sem_init_val(&sema,0);
	h2_sem_init_val(&semb,0);
	if (h2_thread_create(worker_thread, &stack_space[THREAD_STACK_SIZE], 0, 4) == -1) {
		FAIL("Could not create thread\n");
	}
	h2_sem_down(&sema);
	for (i = 0; i < 100000; i++) {
		asm volatile (" nop ");
	}
	h2_sem_up(&semb);
	h2_sem_down(&sema);
	puts("TEST PASSED");
	exit(0);
}

