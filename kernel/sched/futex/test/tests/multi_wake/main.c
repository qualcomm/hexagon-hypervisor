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
 * - multiple wait-and-resume works; waking n threads
 * - no threads are woken if nr = 0 or no threads ready
 */

/*  this test can be reseeded  */

#ifndef TEST_SEED
#define TEST_SEED 2
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

unsigned int done=0;     /*  signals done to main() */
unsigned int counter=0;  /*  global test counter  */
unsigned int age_to_wake=0;
unsigned int max_consumers=0;
unsigned int nr_to_wake=0;

/*  specifically for this test, indexed by consumer thread (age/prio) index  */
unsigned threads_woken[MAX_TEST_THREADS];

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
		error("futex_wake succeeded with nr_to_wake == 0\n");
	}

	for (counter=0; counter<PRODUCER_ITERATIONS; counter++) {
		asm volatile("nop");
	}

	info("counter = %d; Resuming consumer\n", counter);
	h2_futex_wake(&futex_pages[test_lock],nr_to_wake);

	h2_thread_stop();
}

/*  Launch 2 threads, one at "low prio" and one at "high prio"; 
    the resume should wake the higher priority thread  */
void consumer_thread(int age)
{
	info("Consumer started\n");

	/*  remove this section if we get away from "oldest first"  */
	if (age >= nr_to_wake) {
		h2_futex_wait(&futex_pages[test_lock],0);
		error("Inappropriate consumer thread woken up\n");
	}  /*  this consumer should never be woken  */

	h2_futex_wait(&futex_pages[test_lock],0);

	if (counter != PRODUCER_ITERATIONS) {
		error("Wrong final counter value\n");
	}

	threads_woken[age] = 1;
	info("Final counter value:  %d\n",counter);
	done = 1;

	h2_thread_stop();
}

int main() 
{
	unsigned int next_tnum;
	unsigned int timeout=0;
	unsigned int i,j;

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

	/*  
         * min = 1
         * max = max avail threads minus 2 (one for producer, one for main())  
         */
	max_consumers = (rand() % (MAX_TEST_THREADS-MAX_DUMMY_THREADS-2)) + 1;

	/*  
         * min = 1
         * max = max-sleepers
         */

	nr_to_wake = (rand() % (max_consumers-1)) + 1;

	info("test_lock = %d\n",test_lock);
	info("dummy_thread_lock = %d\n",dummy_thread_lock);
	info("max_consumers = %d\n",max_consumers);
	info("nr_to_wake = %d\n",nr_to_wake);

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
			next_tnum,
			0xffffffff) <= 0) {
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

	/*  Spawn off multiple consumer threads waiting for test_lock */

	for (i=0; i<max_consumers; i++) {
		threads_woken[i] = 0;
		if (h2_thread_create(consumer_thread,
			&stack_space[next_tnum++][THREAD_STACK_SIZE],
			(void *)i,
			rand() % 31,
			0xffffffff) <= 0) {
			info("Could not create consumer thread\n");
		}
		/* 
		 * put a small delay in here to make sure the thread launches and
		 * queues into the futex ring in age order; probably more appropriate
		 * to use a lock but whatever.
		 */
		info("delay.\n");
	}

	/*  Producer thread  */
	if (h2_thread_create(producer_thread,
		&stack_space[next_tnum++][THREAD_STACK_SIZE],
		0,
		rand() % 32,
		0xffffffff) <= 0) {
		info("Could not create producer thread\n");
	}

	info("Waiting for done\n");

	while (!done && (timeout < (1<<24))) {
		timeout++;
	}  //  todo:  create watchdog

	/*  give a little settling time  */
	for (timeout=0; timeout<(1<<12)*nr_to_wake; timeout++) {
		asm volatile ("nop");
	}
	if (!done) error("Test timed out\n");

	/*  final check to see if threads_woken == nr_to_wake  */
	j = 0;
	for (i=0; i<max_consumers; i++) {
		info("threads_woken[%d] = %d\n",i,threads_woken[i]);
		if (threads_woken[i]) j++;
	}
	if (j != nr_to_wake) error("wrong # of threads woken; %d vs %d\n",j,nr_to_wake);

	info("TEST PASSED\n");
	return(0);
}

