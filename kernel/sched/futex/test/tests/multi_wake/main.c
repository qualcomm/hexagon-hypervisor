/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <h2.h>
#include <h2_vm.h>
#include <max.h>
#include <tlbfmt.h>
#include <tlbmisc.h>
#include <stdio.h>

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

#define NUM_TOTAL_THREADS (MAX_TEST_THREADS+3)
u64_t			stack_space[MAX_TEST_THREADS][THREAD_STACK_SIZE];

/*  two pages  */
u32_t			futex_pages[LOCK_SLOTS] __attribute__((aligned(4096)));  

/*  index of the dummy thread's lock  */
u32_t			dummy_thread_lock;  

/*  lock under test  */
u32_t			test_lock; 

volatile unsigned int timeout=0;

volatile unsigned int done=0;     /*  signals done to main() */
unsigned int counter=0;  /*  global test counter  */
unsigned int age_to_wake=0;
unsigned int max_consumers=0;
unsigned int nr_to_wake=0;
volatile unsigned int max_prio_to_wake = 0;

/*  priority and done status, indexed by consumer thread counter  */
volatile unsigned int thread_info[MAX_TEST_THREADS];

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	h2_printf(str);
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

	info("counter = %d; Resuming consumer (nr_to_wake=%d)\n", counter,nr_to_wake);
	h2_futex_wake(&futex_pages[test_lock],nr_to_wake);

	h2_thread_stop();
}

void consumer_thread(unsigned int *tinfo)
{
	int prio = *tinfo >> 16;
	info("Consumer started. id=0x%x\n",h2_thread_myid());

	h2_futex_wait(&futex_pages[test_lock],0);

	*tinfo |= 0x1;

	if (counter != PRODUCER_ITERATIONS) {
		error("Wrong final counter value\n");
	}
	if (prio > max_prio_to_wake) {
		error("Inappropriate priority %d woken\n",prio);
	}

	info("%x: %d Final counter value:  %d\n",prio,h2_thread_myid(),counter);
	done = 1;

	h2_thread_stop();
}

void vmmain(void *unused)
{
	unsigned int next_tnum;
	unsigned int i,j,k,prio;
	/*  pick a random lock to use for the dummies  */
	dummy_thread_lock = rand() % (LOCK_SLOTS);
	futex_pages[dummy_thread_lock] = 0;

	printf("VM Main\n");
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

	//h2_config_add_thread_storage(context_space,sizeof(context_space)); 

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
			next_tnum) == -1) {
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
		prio = rand() % 8;
		thread_info[i] = 0 | prio << 16;
		if (h2_thread_create(consumer_thread,
			&stack_space[next_tnum++][THREAD_STACK_SIZE],
			(void *)&thread_info[i],
			prio) == -1) {
			info("Could not create consumer thread\n");
		}
		/* 
		 * put a small delay in here to make sure the thread launches and
		 * queues into the futex ring in age order; probably more appropriate
		 * to use a lock but whatever.
		 */
		info("created consumer thread, prio %d.\n",prio);
	}

	for (i=0; i<max_consumers*2048; i++) {
		asm volatile ("nop;");
	}

	/*  Figure out the max priority that can be woken up  */
	k = nr_to_wake;
	for (j=0; j<MAX_PRIO; j++) {
		for (i=0; i<max_consumers; i++) {
			if ((thread_info[i] >> 16) == j) {
				k--;
			}
			if (k == 0) {
				max_prio_to_wake = j;
				break;
			}

		}
		if (k == 0) {
			break;
		}
	}
	info("Max priority to be woken should be %d\n",max_prio_to_wake);

	/*  Producer thread  */
	if (h2_thread_create(producer_thread,
		&stack_space[next_tnum++][THREAD_STACK_SIZE],
		0,
		rand() % 32) == -1) {
		info("Could not create producer thread\n");
	}

	info("Waiting for done\n");

	while (!done && (timeout < (1<<30))) {
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
		info("thread_info[%d] = %d\n",i,thread_info[i]);
		if (thread_info[i] & 1) j++;
	}
	if (j != nr_to_wake) error("wrong # of threads woken; %d vs %d\n",j,nr_to_wake);

	info("TEST PASSED\n");
	exit(0);
}

#define MAX_SIZE (1024*256)
unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));
unsigned long long int main_thread_stack[THREAD_STACK_SIZE];
void spawn_vm(void *pc)
{
	unsigned int size;
	void *vmb;
	size = h2_config_vmblock_size(NUM_TOTAL_THREADS,1);
	printf("vmblock size: %d\n",size);
	if (size > MAX_SIZE) FAIL("Too much context needed\n");
	vmb = h2_config_vmblock_init(storage,SET_STORAGE_IDENT,0,0);
	printf("vmb: %p\n",vmb);
	vmb = h2_config_vmblock_init(vmb,SET_PMAP_TYPE,0,0);
	h2_config_vmblock_init(vmb,SET_CPUS_INTS,NUM_TOTAL_THREADS,1);
	h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	printf("initted\n");
	h2_vmboot(pc,&main_thread_stack[THREAD_STACK_SIZE-1],0,0,vmb);
	printf("vm booted\n");
}

int main() 
{
	u32_t asid;

	srand(TEST_SEED);

	h2_init(0x0);

	/* set URWX in monitor TLB entry permissions, to allow futex access */
	/* not really necessary to find our asid since the TLB entry will be global
		 anyway, but... */
	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));

	u32_t tlb_index = H2K_mem_tlb_probe(H2K_LINK_ADDR, asid);
	if (tlb_index == 0x80000000) {
		FAIL("Can't find monitor TLB entry");
	}
	u64_t tlb_entry = H2K_mem_tlb_read(tlb_index);
#if __QDSP6_ARCH__ <= 3
	tlb_entry |= 0x7ULL << 29;
#else
	tlb_entry |= 0xfULL << 28;
#endif
	H2K_mem_tlb_write(tlb_index, tlb_entry);

	info("main() starting\n");
	spawn_vm(vmmain);
	h2_thread_stop();
	return 0;
}

