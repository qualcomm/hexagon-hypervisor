/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

#define MAX_THREADS 4
#define STACKSIZE 64

#ifdef DEBUG
#define ITERS (10)
#else
#define ITERS (3000000)
#endif

h2_sem_t tomain;
h2_barrier_t barrier;

unsigned long long int stacks[MAX_THREADS][STACKSIZE];

volatile int ctrs[MAX_THREADS];

void spin(void *arg) {

	register int ctr = ITERS;
	unsigned long long start, end;

	h2_barrier_wait(&barrier);

	printf("Start thread %d\n", (int)arg);
	start = h2_get_tcycles();
	while (ctr--) {
		//ctrs[(int)arg]++;
		asm volatile (" nop" : : : "memory");
	}
	end = h2_get_tcycles();

	//printf("Thread %d:  tcycles:  %d  ctr:  %d\n", (int)arg, end - start, ctrs[(int)arg]);
	h2_sem_up(&tomain);

	h2_thread_stop();
}

int main() {

	unsigned long long start, end;
	int i;

	h2_sem_init_val(&tomain, 0);
	h2_barrier_init(&barrier, MAX_THREADS);
	
	for (i = 0; i < MAX_THREADS; i++) {
		h2_thread_create(spin, &stacks[i][STACKSIZE], (void *)i, i);
	}

	printf("Start\n");
	start = h2_get_core_pcycles();
	h2_sem_down(&tomain);
	end = h2_get_core_pcycles();

	/* for (i = 0; i < MAX_THREADS; i++) { */
	/* 	printf("Thread %d:  count:  %d\n", i, ctrs[i]); */
	/* } */
	printf("\nTotal pcycles:  %lld\n", end - start);

	return 0;
}
