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
#include <globals.h>

#define STACK_SIZE 128

h2_allsignal_t all_threads, all_done;
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
	h2_allsignal_signal(&all_threads, 0xfffffffc);
	if((all_threads.waiting & 0x1) != 0x1) { FAIL("allsignal acknowledged the wrong signal!"); }
	h2_allsignal_signal(&all_threads, 1);
	h2_thread_stop();
}

void thread1(int thread)
{
	h2_printf("thread1 delay\n");
	h2_allsignal_signal(&all_threads, 0xfffffffc);
	if((all_threads.waiting & 0x2) != 0x2) { FAIL("allsignal acknowledged the wrong signal!"); }
	h2_allsignal_signal(&all_threads, 0x2);
	h2_allsignal_wait(&all_done, 0x80000000);
	if(all_done.waiting != 0) { FAIL("allsignal didn't block while waiting"); }
	h2_thread_stop();
}

int main()
{
	h2_init(NULL);
	h2_config_add_thread_storage(contexts,sizeof(contexts));

	h2_allsignal_init(&all_done);
	h2_allsignal_init(&all_threads);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_allsignal_wait(&all_threads, 0x3);
	if(all_threads.waiting != 0) { FAIL("allsignal didn't block while waiting"); }

	h2_allsignal_signal(&all_done, 0x80000000);

	puts("TEST PASSED\n");
	return 0;
}

