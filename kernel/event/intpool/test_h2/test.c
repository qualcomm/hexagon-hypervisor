/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

atomic_u32_t seen;
h2_sem_t int_received_sem;
volatile int stop_threads;

#define NUM_INT (16)
#define INT_START 48
#define INT_LAST (INT_START + NUM_INT)
#define TIMEOUT_SPINS (1024*1024*4)

// FIXME: investigate the reason of these failures
//#define NUM_INT (1)
//#define INT_START 70  // doesn't work 0x46 0b01000110
//#define INT_START 80  // doesn't work 0x50 0b01010000
//#define INT_START 81  // doesn't work 0x51 0b01010001
//#define INT_START 86  // doesn't work 0x56 0b01010110
//#define INT_START 96  // doesn't work 0x60 0b01100000
//#define INT_START 128 // doesn't work 0x80 0b10000000

//#define USE_DETACHED_THREADS

void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

void *timeout(void *arg)
{
	int i;
	h2_printf("time-out thread started\n");
	for (i = 0; i < TIMEOUT_SPINS && !stop_threads; i++) {
		asm volatile ("nop");
	}
	if (stop_threads)
		h2_printf("time-out thread stopped by main thread\n");
	else
		FAIL("time-out");

	return NULL; // NOTE: 	pthread_exit() executed automatically in pthread_trampoline() function
}

atomic_u32_t calc_expected_mask() {
	atomic_u32_t res = 0;
	for (int intno = INT_START; intno < INT_LAST; intno++)
		res |= (1<<(intno & 0x1f));
	return res;
}

void *worker(void *arg)
{
	int intno = -1;
	unsigned int *local_seen = arg;
	while (1 && !stop_threads) {
		if ((intno = h2_intpool_wait(intno)) == -1) FAIL("intpool wait failed");
		h2_printf("saw %d (arg = %x)\n",intno, arg);
		h2_atomic_setbit32(&seen,(intno & 0x1f));
		*local_seen |= (1<<(intno & 0x1f));
		h2_sem_up(&int_received_sem);
	};
	if (stop_threads)
		h2_printf("worker thread is stopped by main thread (arg = %x)\n", arg);

	return NULL; // NOTE: 	pthread_exit() executed automatically in pthread_trampoline() function
} 

struct sched_param lowprio_param = { .sched_priority = 250 };

int main() 
{
	pthread_t timeout_child;
	pthread_t intpool_child_0;
	pthread_t intpool_child_1;
	pthread_attr_t pt_attrs;
	h2_sem_init_val(&int_received_sem,0);
	volatile unsigned int seen_0;
	volatile unsigned int seen_1;
	int i;
	atomic_u32_t expected_mask;

	h2_atomic_swap32(&seen, 0); //seen = 0;
	stop_threads = 0;
	h2_handle_errors(1);
	expected_mask = calc_expected_mask();
	puts("Starting");
	h2_printf("&seen_0 = %x, &seen_1 = %x\n", &seen_0, &seen_1);

	/* Create timeout thread? */
	pthread_attr_init(&pt_attrs);
	pthread_attr_setschedparam(&pt_attrs,&lowprio_param);
	pthread_create(&timeout_child,&pt_attrs,timeout,NULL);
	/* Configure interrupts as intpool interrupts */
	for (i = INT_START; i < INT_LAST; i++) {
		h2_intpool_config(i,1);
	}
	/* Create one intpool thread */
	pthread_create(&intpool_child_0,NULL,worker,(void*)&seen_0);
	/* Trigger interrupt */
	h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE,INT_START,1);
	/* Check that intpool thread (down sem?) happened */
	while (h2_atomic_sub32(&seen, 0) == 0) {
		asm volatile ("nop");
	}
	puts("seen != 0");
	h2_sem_down(&int_received_sem);
	puts("awake!");
	if (h2_atomic_sub32(&seen, 0) == 0) FAIL("didn't see interrupt"); else puts("Test interrupt is ok.\n");
	/* Trigger MOAR INTERRUPTS */
	h2_atomic_swap32(&seen, 0); //seen = 0;
	for (i = INT_START; i < INT_LAST; i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
		h2_printf("send %d!\n",i);
	}
	h2_printf("%d interrupts are sent\n", NUM_INT);
	for (i = INT_START; i < INT_LAST; i++) {
		h2_sem_down(&int_received_sem);
		h2_printf("awake %d!\n",i);
	}
	h2_printf("%d sems down\n", NUM_INT);
	if (h2_atomic_sub32(&seen, 0) != expected_mask) {
		h2_printf("seen=%x  expected seen=%x\n",seen, expected_mask);
		FAIL("what has been seen cannot be unseen");
	}
	seen_0 = seen_1 = 0;
	h2_atomic_swap32(&seen, 0); // seen = 0;
	puts("OK");

	/* Create another intpool thread */
	pthread_create(&intpool_child_1,NULL,worker,(void*)&seen_1);
	/* Check that both intpool threads can get interrupted */
	/* Trigger more interrupts */
	for (i = INT_START; i < INT_LAST; i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
	}
	for (i = INT_START; i < INT_LAST; i++) {
		h2_sem_down(&int_received_sem);
	}
	if (seen_0 == 0) FAIL("only t1 got ints?");
	if (seen_1 == 0) FAIL("only t0 got ints?");
	if ((seen_1 ^ seen_0) != expected_mask) {
		h2_printf("1: %08x  2: %08x  expected_mask: %08x\n",seen_1,seen_0, expected_mask);
		FAIL("bad masks");
	}
	if (h2_atomic_sub32(&seen, 0) != expected_mask) FAIL("what has been seen cannot be unseen");
	seen_0 = seen_1 = 0;

	for (i = INT_START; i < INT_LAST; i++) {
		h2_intpool_config(i,0);
	}
	for (i = INT_START; i < INT_LAST; i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
	}
	for (i = 0; i < 1024; i++) { asm volatile (""); }
	if (seen_0 || seen_1) FAIL("should not have seen anything after disabling.");

	puts("TEST PASSED");
	return 0;
}

