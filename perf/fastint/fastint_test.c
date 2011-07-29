/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

/*
 * This test runs background threads that continually bump counters.
 * A fast interrupt handler also bumps a counter when an interrupt comes in.
 * Once the "end" interrupt is asserted, we read out the counters and print results.
 * Threads that increment their counters less are running less.
 */

#define MAX_THREADS 8
#define STACKSIZE 500

#ifdef H2_H
#define FASTINT_RETURN_TYPE int
#define FASTINT_RETURN(X) return X;
#else
#define FASTINT_RETURN_TYPE void
#define FASTINT_RETURN(X) return;
#endif

unsigned int fastint_count = 0;

volatile unsigned int background_counts[MAX_THREADS];

h2_sem_t startsem;
h2_sem_t donesem;

FASTINT_RETURN_TYPE tick_interrupt(int intno)
{
	fastint_count++;
	FASTINT_RETURN(1);
}

FASTINT_RETURN_TYPE done_interrupt(int intno)
{
	h2_sem_up(&donesem);
	FASTINT_RETURN(1);
}

void background_thread(void *vid)
{
	int id = (int)vid;
	volatile unsigned int *counter = background_counts+id;
	printf("backgroud thread %d started", id);
	h2_sem_down(&startsem);
	while (1) {
		(*counter)++;
	}
}

char thread_storage[(MAX_THREADS+2)*256];

unsigned long long int stacks[MAX_THREADS][STACKSIZE];

int main()
{
	int i;
	unsigned int copied_counts[MAX_THREADS];
	h2_config_add_thread_storage(thread_storage,sizeof(thread_storage));
	h2_register_fastint(4,tick_interrupt);
	h2_register_fastint(5,done_interrupt);
	h2_sem_init_val(&donesem,0);
	h2_sem_init_val(&startsem,0);
	for (i = 0; i < MAX_THREADS; i++) background_counts[i] = 0;
	for (i = 0; i < MAX_THREADS; i++) {
		h2_thread_create((void *)background_thread,&stacks[i][STACKSIZE],(void *)i,32+i);
	}
	h2_sem_add(&startsem,MAX_THREADS);
	h2_sem_down(&donesem);
	for (i = 0; i < MAX_THREADS; i++) {
		copied_counts[i] = background_counts[i];
	}
	printf("Fast Interrupts: %d\n",fastint_count);
	for (i = 0; i < MAX_THREADS; i++) {
		printf("T%d iters: %d\n",i,copied_counts[i]);
	}
	return 0;
}

