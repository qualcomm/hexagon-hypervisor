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

#define TIMEOUT_SPINS (1024*1024*4)
#define INT_START 48
#define INT_LAST 128

void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

void *timeout(void *arg)
{
	int i;
	h2_printf("timeout thread started\n");
	for (i = 0; i < TIMEOUT_SPINS; i++) {
		asm volatile ("nop");
	}
	FAIL("timeout");
	return NULL;
}

void *worker(void *arg)
{
	int intno = -1;
	unsigned int *local_seen = arg;
	while (1) {
		if ((intno = h2_intpool_wait(intno)) == -1) FAIL("intpool wait failed");
		// h2_printf("saw %d\n",intno);
		h2_atomic_setbit32(&seen,(intno & 0x1f));
		*local_seen |= (1<<(intno & 0x1f));
		h2_sem_up(&int_received_sem);
	};
	return NULL;
} 

struct sched_param lowprio_param = { .sched_priority = 250 };

int main() 
{
	pthread_t timeout_child;
	pthread_t intpool_child_0;
	pthread_t intpool_child_1;
	pthread_attr_t pt_attrs;
	h2_sem_init_val(&int_received_sem,0);
	unsigned int seen_0;
	unsigned int seen_1;
	int i;
	

	seen = 0;
	h2_handle_errors(1);
	puts("Starting");

	/* Create timeout thread? */
	pthread_attr_init(&pt_attrs);
	pthread_attr_setschedparam(&pt_attrs,&lowprio_param);
	pthread_create(&timeout_child,&pt_attrs,timeout,NULL);
	/* Configure interrupts as intpool interrupts */
	for (i = INT_START; i < INT_LAST; i++) {
		h2_intpool_config(i,1);
	}
	/* Create one intpool thread */
	pthread_create(&intpool_child_0,NULL,worker,&seen_0);
	/* Trigger interrupt */
	h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE,INT_START,1);
	/* Check that intpool thread (down sem?) happened */
	h2_sem_down(&int_received_sem);
	h2_printf("awake!\n");
	if (seen == 0) FAIL("didn't see interrupt");
	/* Trigger MOAR INTERRUPTS */
	seen = 0;
	for (i = INT_START; i < (INT_START+32); i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
	}
	for (i = INT_START; i < (INT_START+32); i++) {
		h2_sem_down(&int_received_sem);
		// h2_printf("awake %d!\n",i);
	}
	if (seen != ~0) {
		h2_printf("seen=%x\n",seen);
		FAIL("what has been seen cannot be unseen");
	}
	seen_0 = seen_1 = seen = 0;
	puts("OK");

	/* Create another intpool thread */
	pthread_create(&intpool_child_1,NULL,worker,&seen_1);
	/* Check that both intpool threads can get interrupted */
	/* Trigger more interrupts */
	for (i = INT_START; i < (INT_START+32); i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
	}
	for (i = INT_START; i < (INT_START+32); i++) {
		h2_sem_down(&int_received_sem);
	}
	if (seen_0 == 0) FAIL("only t1 got ints?");
	if (seen_1 == 0) FAIL("only t0 got ints?");
	if ((seen_1 ^ seen_0) != ~0) {
		h2_printf("1: %08x 2: %08x\n",seen_1,seen_0);
		FAIL("bad masks");
	}
	if (seen != ~0) FAIL("what has been seen cannot be unseen");
	seen_0 = seen_1 = seen = 0;

	for (i = INT_START; i < INT_LAST; i++) {
		h2_intpool_config(i,0);
	}
	for (i = INT_START; i < (INT_START+32); i++) {
		h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE, i, 1);
	}
	for (i = 0; i < 1024; i++) { asm volatile (""); }
	if (seen_0 || seen_1) FAIL("should not have seen anything after disabling.");

	puts("TEST PASSED");
	return 0;
}

