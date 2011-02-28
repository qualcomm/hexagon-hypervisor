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

#define SPINS (1024*1024)
#define STACK_SIZE 128

h2_sem_t sem_call,sem_ret,sem_done;

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

void thread0(int thread)
{
	int i;
	H2K_thread_context *me;
#if __QDSP6_ARCH__ <= 3
	asm ( " %0 = sgp " : "=r"(me));
#else
	asm ( " %0 = sgp0 " : "=r"(me));
#endif
	u64_t startpcycles;
	u64_t endpcycles;
	u64_t startcputime;
	u64_t endcputime;
	s64_t delta;
	startpcycles = H2K_pcycles_get(me);
	startcputime = H2K_cputime_get(me);
	for (i = 0; i < 128; i++) {
		endpcycles = H2K_pcycles_get(me);
		endcputime = H2K_cputime_get(me);
		delta = (endpcycles - startpcycles) - (endcputime - startcputime);
		if (delta < 0) delta = -delta;
		if (delta > 32) FAIL("cputime/pcycles diverging");
		if (endpcycles <= startpcycles) FAIL("pcycles not incrementing");
		if (endcputime <= startcputime) FAIL("cputime not incrementing");
	}
	startpcycles = H2K_pcycles_get(me);
	startcputime = H2K_cputime_get(me);
	delay();
	endpcycles = H2K_pcycles_get(me);
	endcputime = H2K_cputime_get(me);
	delta = (endpcycles - startpcycles) - (SPINS * MAX_HTHREADS);
	if (delta < 0) delta = -delta;
	if (delta > 64 * MAX_HTHREADS) FAIL("Unexpected delta based on delay (A) ");
	delta = (endcputime - startcputime) - (SPINS * MAX_HTHREADS);
	if (delta < 0) delta = -delta;
	if (delta > 64 * MAX_HTHREADS) FAIL("Unexpected delta based on delay (B) ");

	startpcycles = H2K_pcycles_get(me);
	startcputime = H2K_cputime_get(me);
	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	endpcycles = H2K_pcycles_get(me);
	endcputime = H2K_cputime_get(me);
	delta = (endpcycles - startpcycles) - (SPINS * MAX_HTHREADS);
	if (delta < 0) delta = -delta;
	if (delta > 1024 * MAX_HTHREADS) FAIL("Unexpected delta based on delay (C) ");
	delta = (endcputime - startcputime);
	if (delta < 0) delta = -delta;
	if (delta > 1024 * MAX_HTHREADS) FAIL("Unexpected delta based on delay (D) ");

	h2_sem_up(&sem_done);
	h2_thread_stop();
}

void thread1(int thread)
{
	h2_sem_down(&sem_call);
	delay();
	h2_sem_up(&sem_ret);
	h2_thread_stop();
}

int main()
{
	h2_init(NULL);
	h2_config_add_thread_storage(contexts,sizeof(contexts));
	h2_sem_init_val(&sem_call,0);
	h2_sem_init_val(&sem_ret,0);
	h2_sem_init_val(&sem_done,0);
	h2_thread_create(thread0,&stack0[STACK_SIZE],0,2,0xffffffff);
	h2_thread_create(thread1,&stack1[STACK_SIZE],0,2,0xffffffff);
	h2_sem_down(&sem_done);
	puts("TEST PASSED\n");
	return 0;
}

