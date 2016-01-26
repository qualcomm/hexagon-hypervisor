/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <h2.h>
//#include <globals.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define ITERS 100

// #define SIMULATOR_PMU_SUPPORT 1

h2_sem_t sem_a,sem_b,sem_c,sem_done;
int t0id,t1id;

unsigned long long int stack0[STACK_SIZE];
unsigned long long int stack1[STACK_SIZE];

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void thread0(int thread)
{
	int i;
	h2_sem_add(&sem_c,1);
	for (i = 0; i < (ITERS-1); i++) {
		h2_sem_down(&sem_a);
	}
	if (h2_sem_trydown(&sem_a) != 0) {
		FAIL("trydown failed\n");
	}
	if (h2_sem_trydown(&sem_a) == 0) {
		FAIL("trydown succeeded\n");
	}
	h2_sem_up(&sem_done);
	h2_thread_stop(0);
}

void thread1(int thread)
{
	h2_sem_up(&sem_c);
	h2_sem_down(&sem_b);
	h2_sem_up(&sem_done);
	h2_thread_stop(0);
}

int main()
{
//	h2_init(NULL);

	h2_sem_init_val(&sem_done,0);
	h2_sem_init_val(&sem_a,0);
	h2_sem_init_val(&sem_b,0);
	h2_sem_init(&sem_c);

	h2_sem_down(&sem_c);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_sem_down(&sem_c);
	h2_sem_down(&sem_c);

	h2_sem_add(&sem_a,ITERS);

	h2_sem_down(&sem_done);
	h2_sem_up(&sem_b);
	h2_sem_down(&sem_done);
	puts("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}

