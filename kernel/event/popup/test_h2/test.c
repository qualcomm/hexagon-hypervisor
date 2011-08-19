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
#include <hw.h>
#include <globals.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

// #define SIMULATOR_PMU_SUPPORT 1

h2_sem_t sem_call,sem_ret,sem_done;
int t0id,t1id;

u64_t stack0[STACK_SIZE];
u64_t stack1[STACK_SIZE];

u64_t contexts[3*sizeof(H2K_thread_context)/sizeof(u64_t)] __attribute__((aligned(32)));

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void delay()
{
	asm volatile (
	" jump 2f\n"
	" .p2align 4\n"
	"2: \n"
	" loop0(1f,%0) \n"
	"1:\n"
	" { nop; }:endloop0 \n" : : "r"(SPINS) :"lc0");
}

static inline int myabs(int x)
{
	if (x < 0) {
		return -x; 
	} else return x;
}

#define INTERRUPT_T0 12
#define INTERRUPT_T1 13

static volatile int counter0;
static volatile int counter1;

h2_sem_t sem0;
h2_sem_t sem1;
h2_sem_t startsem;

/*
 * Strategy 
 * Each thread blocks, bumps a counter, downs a semaphore, and then loops back to block again
 * Every time I set an interrupt, I should be able to watch the thread bump the counter
 * By bumping the semaphore first, and then the interrupt, or vice-versa, I can cause the 
 * interrupt to already be pending when the thread tries to block, or not pending.
 */

void thread0(int thread)
{
	printf("T0 started\n");
	h2_sem_up(&startsem);
	while (1) {
		h2_intwait(INTERRUPT_T0);
		counter0++;
		printf("T0\n");
		h2_sem_down(&sem0);
	}
	h2_thread_stop();
}

void thread1(int thread)
{
	printf("T1 started\n");
	h2_sem_up(&startsem);
	while (1) {
		h2_intwait(INTERRUPT_T1);
		counter1++;
		printf("T1\n");
		h2_sem_down(&sem1);
	}
	h2_thread_stop();
}

void gen_int(int which)
{
	int intno = INTERRUPT_T0 + which;
#if ARCHV >= 4
	swi(1<<intno);
#else
	swi(0x80000000UL>>intno);
#endif
}

int main()
{
	h2_init(NULL);
	h2_config_add_thread_storage(contexts,sizeof(contexts));

	counter0 = counter1 = 0;

	h2_sem_init_val(&sem0,0);
	h2_sem_init_val(&sem1,0);
	h2_sem_init_val(&startsem,0);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_sem_down(&startsem);
	h2_sem_down(&startsem);
	delay();

	if (counter0 != 0) FAIL("Bad counter0: should be 0\n");
	if (counter1 != 0) FAIL("Bad counter1: should be 0\n");

	gen_int(0);
	delay();

	if (counter0 != 1) FAIL("Bad counter0: should be 1\n");
	if (counter1 != 0) FAIL("Bad counter1: should be 0\n");

	gen_int(1);
	delay();

	if (counter0 != 1) FAIL("Bad counter0: should be 1\n");
	if (counter1 != 1) FAIL("Bad counter1: should be 1\n");

	gen_int(0);
	delay();

	if (counter0 != 1) FAIL("Bad counter0: should be 1\n");
	if (counter1 != 1) FAIL("Bad counter1: should be 1\n");

	h2_sem_up(&sem0);
	h2_sem_up(&sem1);
	delay();

	if (counter0 != 2) FAIL("Bad counter0: should be 2\n");
	if (counter1 != 1) FAIL("Bad counter1: should be 1\n");

	gen_int(1);
	delay();

	if (counter0 != 2) FAIL("Bad counter0: should be 2\n");
	if (counter1 != 2) FAIL("Bad counter1: should be 2\n");

	puts("TEST PASSED\n");
	return 0;
}

