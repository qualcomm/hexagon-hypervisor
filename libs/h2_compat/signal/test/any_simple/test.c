/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>
//#include <globals.h>

#define STACK_SIZE 128

h2_signal_t any_threads, any_done;
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

void thread0(int thread)
{
	h2_printf("thread0 delay\n");
	h2_signal_set(&any_threads, 0xfffffffc);
	//if((any_threads.mask & 0x1) != 0x1) { FAIL("anysignal acknowledged the wrong signal!"); }
	h2_signal_set(&any_threads, 1);
	h2_thread_stop(0);
}

void thread1(int thread)
{
	h2_printf("thread1 delay\n");
	h2_signal_wait_any(&any_done, 0xfffffff0);
	h2_thread_stop(0);
}

int main()
{
	unsigned int sig;

//	h2_init(NULL);

	h2_signal_init(&any_done);
	h2_signal_init(&any_threads);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_signal_wait_any(&any_threads, 0x3);
	//if(any_threads.signals != 0) { FAIL("anysignal didn't block while waiting"); }

	h2_signal_set(&any_done, 0x1);
	h2_signal_set(&any_done, 0x2);
	h2_signal_set(&any_done, 0x4);
	h2_signal_set(&any_done, 0x8);
	h2_signal_set(&any_done, 0xf);
	sig = h2_signal_get(&any_done);
	if(sig != 0xf) { FAIL("anysignal acknowledged the wrong signal(s)!"); }
	h2_signal_set(&any_done, 0x8000);

	puts("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}

