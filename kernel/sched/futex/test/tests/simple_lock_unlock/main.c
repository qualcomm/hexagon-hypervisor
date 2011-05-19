/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <h2.h>

/*
 * This test checks the following functionality:
 * 
 * - futex_wait fails on an invalid lock
 * - simple wait-and-resume works; waking 1 thread 
 *   will wake highest priority thread
 * - no threads are woken
 */

/*  this test can be reseeded  */

#ifndef TEST_SEED
#define TEST_SEED 1
#endif

#define PRODUCER_ITERATIONS (1<<14)

#define MAX_DUMMY_THREADS 32
#define MAX_TEST_THREADS  64
#define THREAD_STACK_SIZE 256
#define LOCK_PAGES 2

#ifndef PAGE_SIZE
#define PAGE_SIZE 1<<12
#endif

#ifndef BYTES_PER_LONG
#define BYTES_PER_LONG 4
#endif

#define LOCK_SLOTS (LOCK_PAGES*(PAGE_SIZE/BYTES_PER_LONG))

H2K_thread_context	context_space[MAX_TEST_THREADS];
u64_t			stack_space[MAX_TEST_THREADS][THREAD_STACK_SIZE];

/*  two pages  */
u32_t			futex_pages[LOCK_SLOTS] __attribute__((aligned(4096)));  

/*  index of the dummy thread's lock  */
u32_t			dummy_thread_lock;  

/*  lock under test  */
u32_t			test_lock; 

unsigned int counter=0;
volatile unsigned int done=0;

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	exit(1);
}

void dummy_thread(int x)
{
	//info("dummy lock = %d\n",futex_pages[dummy_thread_lock]);
	h2_futex_wait(&futex_pages[dummy_thread_lock],0);
	error("Somebody woke up a dummy thread\n");
	h2_yield();
}

/*  Producer thread just counts up and then wakes up the other thread(s)  */

void producer_thread(int x)
{
	info("Producer started\n");

	/*  "two for flinching"  */
	if (h2_futex_wake(&futex_pages[test_lock],0) != 0) {
		error("futex_wake succeeded with n_to_wake == 0\n");
	}

	for (counter=0; counter<PRODUCER_ITERATIONS; counter++) {
		asm volatile("nop");
	}
	info("counter = %d; Resuming consumer\n", counter);
	h2_futex_wake(&futex_pages[test_lock],1);

	h2_thread_stop();
}

/*  Launch 2 threads, one at "low prio" and one at "high prio"; 
    the resume should wake the higher priority thread  */
void consumer_thread(int prio)
{
	info("Consumer started\n");

	if (prio == 31) {
		h2_futex_wait(&futex_pages[test_lock],0);
		error("Low priority consumer thread woken up\n");
	}  /*  this consumer should never be woken  */

	h2_futex_wait(&futex_pages[test_lock],0);

	if (counter != PRODUCER_ITERATIONS) {
		error("Wrong final counter value\n");
	}

	info("Final counter value:  %d\n",counter);
	done = 1;

	h2_thread_stop();
}

int main() 
{
	unsigned int next_tnum;
	unsigned int timeout=0;

	srand(TEST_SEED);

	h2_init(0x0);

	info("main() starting\n");

	/*  pick a random lock to use for the dummies  */
	dummy_thread_lock = rand() % (LOCK_SLOTS);
	futex_pages[dummy_thread_lock] = 0;

	do {
		test_lock = rand() % (LOCK_SLOTS);
	} while (test_lock == dummy_thread_lock);

	futex_pages[test_lock] = 0;

	info("test_lock = %d\n",test_lock);
	info("dummy_thread_lock = %d\n",dummy_thread_lock);

	h2_config_add_thread_storage(context_space,sizeof(context_space)); 

	/*  Check that the futex should fail first  */

	if (h2_futex_wait(&futex_pages[test_lock],1) != -1) {
		/*
		 * Actually, this will never reach here and 
		 * should timeout if it acquires the lock  
		 */
		error("Locked on a bad value\n");
	}

	/*  start "dummy" threads ranging in priority from lowest to highest  */
	for (next_tnum=0; next_tnum<MAX_DUMMY_THREADS; next_tnum++) {
		info("Creating dummy thread %d\n",next_tnum);
		if (h2_thread_create(dummy_thread,
			&stack_space[next_tnum][THREAD_STACK_SIZE],
			0,
			next_tnum) <= 0) {
			info("Could not create thread\n");
		}
	}

	/*  "two for flinching"  */
	if (h2_futex_wake(&futex_pages[test_lock],1) != 0) {
		error("futex_wake returned n_woken with empty queues\n");
	}

	/*  "even more for flinching"  */
	if (h2_futex_wake(&futex_pages[test_lock],2) != 0) {
		error("futex_wake returned n_woken with empty queues\n");
	}

	/*  High prio thread  */
	debug("next_tnum = %d\n",next_tnum);
	if (h2_thread_create(consumer_thread,
		&stack_space[next_tnum++][THREAD_STACK_SIZE],
		0,
		rand() % 31) <= 0) {
		info("Could not create consumer thread\n");
	}

	/*  Low prio thread  */
	debug("next_tnum = %d\n",next_tnum);
	if (h2_thread_create(consumer_thread,
		&stack_space[next_tnum++][THREAD_STACK_SIZE],
		(void *)31,
		31) <= 0) {
		info("Could not create 2nd consumer thread\n");
	}

	/*  Producer thread  */
	debug("next_tnum = %d\n",next_tnum);
	if (h2_thread_create(producer_thread,
		&stack_space[next_tnum++][THREAD_STACK_SIZE],
		0,
		rand() % 32) <= 0) {
		info("Could not create producer thread\n");
	}

	info("Waiting for done\n");

	while (!done && (timeout < (1<<20))) {
		timeout++;
	}  //  todo:  create watchdog

	/*  give a little settling time  */
	for (timeout=0; timeout<(1<<12); timeout++) {
		asm volatile ("nop");
	}
	if (!done) error("Test timed out\n");

	info("TEST PASSED\n");
	return(0);
}

