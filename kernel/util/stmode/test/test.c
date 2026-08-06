/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <max.h>
#include <stmode.h>
#include <hw.h>
#include <h2.h>
#include <context.h>
#include <globals.h>
#include <thread.h>
#include <hwconfig.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int handshake = 0;

#define STACK_SIZE 16
#define NUM_TEST_THREADS 2  /* test1 + test2 */

unsigned long long int stack1[STACK_SIZE];
unsigned long long int stack2[STACK_SIZE];

/* Extra thread contexts for test1/test2.  The boot VM is created with
   MAX_BOOT_CONTEXTS=1 (just main), leaving no free slots.  We extend
   the free list with two statically-allocated contexts before creating
   any threads. */
static H2K_thread_context extra_contexts[NUM_TEST_THREADS] __attribute__((aligned(32)));

static inline void stmode_extend_bootvm_contexts(H2K_thread_context *me)
{
	H2K_vmblock_t *vm = me->vmblock;
	int i;
	for (i = 0; i < NUM_TEST_THREADS; i++) {
		H2K_thread_context_clear(&extra_contexts[i]);
		extra_contexts[i].id.raw = 0;
		extra_contexts[i].id.vmidx = vm->vmidx;
		extra_contexts[i].id.cpuidx = vm->max_cpus++;
		extra_contexts[i].vmblock = vm;
		extra_contexts[i].next = vm->free_threads;
		vm->free_threads = &extra_contexts[i];
	}
}

static inline void wait_for_threads_to_sleep(void)
{
	u32_t mc;
	do {
		mc = H2K_get_modectl();
	} while ((mc & ((~mc) >> MODECTL_W_BITS)) & ~0x1u);
}

void test1(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" wait(r0)\n");
	}
}

void test2(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" nop\n");
	}
}

int main()
{
	h2_init(NULL);
	H2K_thread_context *me = H2K_get_sgp();
	u32_t tmp;

	stmode_extend_bootvm_contexts(me);
	H2K_trap_hwconfig_hwthreads_mask(0, NULL, (u32_t)-1, 0, me);
	wait_for_threads_to_sleep();

	H2K_set_gie();

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread active)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	h2_thread_create(test1, &stack1[STACK_SIZE], (void *)1, 0);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread sleep)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	handshake = 0;

	h2_thread_create(test2, &stack2[STACK_SIZE], (void *)2, 0);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp == 0) FAIL("stmode passed (2 threads active)");
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_begin didn't reenable gie");

	puts("TEST PASSED");
	return 0;
}
