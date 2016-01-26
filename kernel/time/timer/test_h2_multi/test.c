/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <cputime.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (256)
#define STACK_SIZE 256
#define N_THREADS 12

#define MAX_DELTA 200000

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int saw_interrupt = 0;

extern u64_t h2_time_get_time();
extern u64_t h2_time_set_timeout(u64_t timeout);
extern u64_t h2_time_get_timeout();
extern void set_vectors();

u64_t stacks[STACK_SIZE*N_THREADS];

h2_sem_t donesem;

#define RANDOM_SHIFT 14
#define RANDOM_BASE 50000 /* .05 MSEC */

static inline u64_t random_delta(u64_t seed) {
	return (((u32_t)seed * 2654435769U) >> RANDOM_SHIFT) + RANDOM_BASE;
}

void task(void *arg)
{
	s64_t start, wakeup, wakeup_real, end, delta;
	int i,ret;
	set_vectors();
	printf("Task started. id=%p\n",arg);
	for (i = 0; i < SPINS; i++) {
		h2_vmtrap_setie(1);
		h2_vmtrap_intop(H2K_INTOP_GLOBEN,12,0);
		start = h2_time_get_time();
		//h2_printf("start=%llx\n",start);
		wakeup = start + random_delta(start);
		if (h2_time_set_timeout(wakeup) == 0) {
			FAIL("timeout in past");
		}
		if (abs((wakeup_real = h2_time_get_timeout()) - wakeup) > 100) {
			h2_printf("wakeup=%llx wakeup_real=%llx\n",wakeup,wakeup_real);
		}
		if ((ret = h2_futex_wait(&i,i)) != 0) {
			/* EJP: FIXME: is this when a thread does vmwork after vmwait? */
			//h2_printf("t=%p id=%x futex wait: %x\n",arg,h2_thread_myid(),ret);
		}
		end = h2_time_get_time();
		delta = end - wakeup;
		if (delta < 0) delta = -delta;
		//printf("id=%p: start=%llx wakeup=%llx end=%llx\n",arg,start,wakeup,end);
		if (delta > MAX_DELTA) {
			printf("delta: %llu\n",delta);
			FAIL("Delta too big");
		}
	};
	h2_sem_up(&donesem);
	h2_thread_stop(0);
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

