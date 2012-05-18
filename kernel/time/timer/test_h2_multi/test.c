/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024)
#define STACK_SIZE 256
#define N_THREADS 12

#define MAX_DELTA 100000

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int saw_interrupt = 0;

extern u64_t h2_time_get_time();
extern u64_t h2_time_set_timeout(u64_t timeout);
extern void set_vectors();

u64_t stacks[STACK_SIZE*N_THREADS];

h2_sem_t donesem;

#define RANDOM_SHIFT 14
#define RANDOM_BASE 100000 /* .1 MSEC */

static inline u64_t random_delta(u64_t seed) {
	return (((u32_t)seed * 2654435769U) >> RANDOM_SHIFT) + RANDOM_BASE;
}

void task(void *arg)
{
	u64_t start, wakeup, end, delta;
	int i;
	set_vectors();
	printf("Task started. id=%p\n",arg);
	for (i = 0; i < SPINS; i++) {
		h2_vmtrap_setie(1);
		h2_vmtrap_intop(H2K_INTOP_GLOBEN,12,0);
		start = h2_time_get_time();
		wakeup = start + random_delta(start);
		if (h2_time_set_timeout(wakeup) == 0) {
			FAIL("timeout in past");
		}
		if (h2_futex_wait(&i,i) >= 0) FAIL("futex bad");
		end = h2_time_get_time();
		delta = end - wakeup;
		printf("id=%p: start=%llx wakeup=%llx end=%llx\n",arg,start,wakeup,end);
		if (delta > MAX_DELTA) FAIL("Delta too big");
	};
	h2_sem_up(&donesem);
	h2_thread_stop();
}

int main() 
{
	int i;
	h2_sem_init_val(&donesem,0);
	for (i = 0; i < N_THREADS; i++) {
		h2_thread_create(task,&stacks[STACK_SIZE+i*STACK_SIZE], (void *)i, 10);
	}
	for (i = 0; i < N_THREADS; i++) {
		h2_sem_down(&donesem);
	}
	puts("TEST PASSED\n");
	return 0;
}

