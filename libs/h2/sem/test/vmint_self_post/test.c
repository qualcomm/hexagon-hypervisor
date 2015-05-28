/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <h2.h>
#include <h2_common_timer.h>
//#include <globals.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define ITERS 100
#define SLEEP_NS 10000

// #define SIMULATOR_PMU_SUPPORT 1

typedef h2_u64_t u64_t;
typedef h2_u32_t u32_t;

h2_sem_t sem_start,sem_done,sem_work;
int t0id,t1id;

u64_t stack0[STACK_SIZE];
u64_t stack1[STACK_SIZE];

// u64_t contexts[3*sizeof(H2K_thread_context)/sizeof(u64_t)] __attribute__((aligned(32)));

unsigned int interrupt_count;
unsigned int thread_iters;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void thread0(int thread)
{
	int i;
	h2_sem_add(&sem_done,1);

	while (1) {
		h2_sem_down(&sem_start);
		for (i = 0; i < 1000; i++) { asm volatile ("nop"); }
		h2_sem_up(&sem_work);
		thread_iters++;
	};
	h2_thread_stop(0);
}

#if 0
void thread1(int thread)
{
	h2_sem_up(&sem_c);
	h2_sem_down(&sem_b);
	h2_sem_up(&sem_done);
	h2_thread_stop(0);
}
#endif

static inline void set_timer()
{
	h2_vmtrap_timerop(H2K_TIMER_TRAP_DELTA_TIMEOUT, SLEEP_NS);
}

static inline void timer_int_enable()
{
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0);
}

void timer_handler()
{
	//printf("Got Int!\n");
	interrupt_count++;
	// say there's more work
	h2_sem_up(&sem_work);
	// set timeout for ~1000 ns in the future
	set_timer();
	// Re-enable interurpt
	timer_int_enable();
}

void sem_test_vecsetup();

int main()
{
//	h2_init(NULL);
	int i;
	h2_sem_init_val(&sem_done,0);
	h2_sem_init_val(&sem_work,0);
	h2_sem_init_val(&sem_start,0);

	h2_vmtrap_setie(1);
	timer_int_enable();
	set_timer();
	sem_test_vecsetup();

	//t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_sem_down(&sem_done);

	for (i = 0; i < ITERS; i++) {
		h2_sem_down(&sem_work);
		if (i & 1) h2_sem_up(&sem_start);
		h2_sem_down(&sem_work);
	}
	printf("Interrupt count: %d expected approx: %d\n",interrupt_count,ITERS*3/2);
	printf("Thread iters: %d expected approx: %d\n",thread_iters,ITERS/2);
	puts("TEST PASSED\n");
	return 0;
}

