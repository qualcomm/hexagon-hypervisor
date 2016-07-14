/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <cputime.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define SMALL_DELTA 256ULL

h2_sem_t sem_call,sem_ret,sem_done,sem_start;

u64_t stack0[STACK_SIZE];
u64_t stack1[STACK_SIZE];

u64_t contexts[3*sizeof(H2K_thread_context)/sizeof(u64_t)] __attribute__((aligned(32)));

/* OVERHEAD is the rough cycle count to save state, swap to another thread, and swap back */
/* it needs to have enough headroom for -Os and -Os -fno-inline versions of h2.           */

#if ARCHV == 3
#define PCYCLES_PER_TCYCLE 6
#define OVERHEAD 1024
#elif ARCHV == 4
#define PCYCLES_PER_TCYCLE 3
#define OVERHEAD 1024
#elif ARCHV == 5
#define PCYCLES_PER_TCYCLE 3
#define OVERHEAD 1536
#elif ARCHV >= 60
#define PCYCLES_PER_TCYCLE 2
#define OVERHEAD 1536
#else
#error define pcycles per tcycle and overhead
#endif

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

void thread0(int thread)
{
	int i;
	u64_t startpcycles;
	u64_t endpcycles;
	u64_t startcputime;
	u64_t endcputime;
	s64_t delta;
	startpcycles = h2_get_core_pcycles();
	startcputime = h2_get_pcycles();
	//printf("start: %llx/%llx\n",startpcycles,startcputime);
	for (i = 0; i < 1024; i++) {
		endpcycles = h2_get_core_pcycles();
		endcputime = h2_get_pcycles();
		delta = (endpcycles - startpcycles) - (endcputime - startcputime);
		//printf("%llx/%llx (Delta: %lld)\n",endpcycles,endcputime,delta);
		if (delta < 0) delta = -delta;
		if (delta > 128) {
			FAIL("cputime/pcycles diverging");
		}
		if (endpcycles <= startpcycles) FAIL("pcycles not incrementing");
		if (endcputime <= startcputime) FAIL("cputime not incrementing");
	}
	startpcycles = h2_get_core_pcycles();
	startcputime = h2_get_pcycles();
	delay();
	endpcycles = h2_get_core_pcycles();
	endcputime = h2_get_pcycles();
	delta = (endpcycles - startpcycles) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = -delta;
	printf("delta 1 = %lld,pcycles=%lld\n",delta,SMALL_DELTA*PCYCLES_PER_TCYCLE);
	if (delta > SMALL_DELTA * PCYCLES_PER_TCYCLE) {
		FAIL("Unexpected delta based on delay (A) ");
	}
	delta = (endcputime - startcputime) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = -delta;
	printf("delta 2 = %lld\n", delta);
	if (delta > SMALL_DELTA * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (B) ");

	startpcycles = h2_get_core_pcycles();
	startcputime = h2_get_pcycles();
	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	endpcycles = h2_get_core_pcycles();
	endcputime = h2_get_pcycles();
	delta = (endpcycles - startpcycles) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = 0;
	if (delta > OVERHEAD * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (C) ");
	printf("delta pcycles: %llx - %x == %llx\n",
		endpcycles-startpcycles,SPINS*PCYCLES_PER_TCYCLE,delta);
	delta = (endcputime - startcputime);
	if (delta < 0) delta = -delta;
	if (delta > OVERHEAD * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (D) ");
	printf("delta cputime: %llx\n",delta);
	h2_sem_up(&sem_done);
	h2_thread_stop(0);
}

void thread1(int thread)
{
	h2_sem_up(&sem_start);
	h2_sem_down(&sem_call);
	delay();
	h2_sem_up(&sem_ret);
	h2_thread_stop(0);
}

int main(int argc, char **argv)
{
	printf("VM Started\n");
	h2_sem_init_val(&sem_start,0);
	h2_sem_init_val(&sem_call,0);
	h2_sem_init_val(&sem_ret,0);
	h2_sem_init_val(&sem_done,0);
	h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);
	h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	h2_sem_down(&sem_done);
	puts("TEST PASSED\n");
	exit(0);
}
