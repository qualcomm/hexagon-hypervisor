/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <h2.h>
#include <h2_vm.h>
#include <h2_config.h>
#include <h2_vmtraps.h>
#include <max.h>
#include <stdio.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

/*
 * This test checks the following functionality:
 * 
 * - futex_wait fails on an invalid lock
 * - multiple wait-and-resume works; waking n threads
 * - no threads are woken if nr = 0 or no threads ready
 */

#define NUM_DUMMY_THREADS 1024
#define THREAD_STACK_SIZE 256
#define NUM_GOOD_THREADS 4
#define MAIN_THREADS 1
#define NUM_TOTAL_THREADS (NUM_DUMMY_THREADS+NUM_GOOD_THREADS+MAIN_THREADS)

u64_t			stack_space[NUM_DUMMY_THREADS+NUM_GOOD_THREADS][THREAD_STACK_SIZE];
u64_t			main_thread_stack[THREAD_STACK_SIZE];

/*  two pages  */
u32_t			dummy_futex_ptrs[NUM_DUMMY_THREADS];

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
volatile unsigned int thread_info[NUM_GOOD_THREADS];

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	h2_printf(str);
	while (1);
	exit(1);
}

void dummy_thread(int x)
{
	//info("dummy lock = %d\n",futex_pages[dummy_thread_lock]);
	if (h2_futex_wait(&dummy_futex_ptrs[x],0) == -1) {
		FAIL("h2_futex_wait");
	}
	FAIL("dummy thread woke");
}

#if 0
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
#endif

void test_thread(unsigned int goodid)
{
	info("Consumer started (%d). id=0x%x\n",goodid,h2_thread_myid());

	thread_info[goodid] = 1;

	while (1) {
		h2_futex_wait(&test_lock,0);
		info("Ping (%d)\n",goodid);
		thread_info[goodid]++;
	}
}

void vmmain(void *unused)
{
	unsigned int i;
	unsigned int flag;
	h2_handle_errors();
	printf("Hello, World!\n");
	printf("TID: %x\n",h2_thread_myid());
	//h2_config_add_thread_storage(context_space,sizeof(context_space));

	for (i = 0; i < NUM_DUMMY_THREADS; i++) {
		if (h2_thread_create(dummy_thread,
			&stack_space[i][THREAD_STACK_SIZE],
			(void *)i, 2) == -1) {
			FAIL("Couldn't create dummy");
		}
	}
	printf("Still alive...\n");

	for (i = 0; i < NUM_GOOD_THREADS; i++) {
		if (h2_thread_create(test_thread,
			&stack_space[i+NUM_DUMMY_THREADS][THREAD_STACK_SIZE],
			(void *)i, 4+i) == -1) {
			FAIL("Couldn't create test thread");
		}
	}

	while (1) {
		flag = 0;
		for (i = 0; i < NUM_GOOD_THREADS; i++) {
			if (thread_info[i] == 0) flag = 1;
		}
		if (flag) {
			continue;
		} else break;
	}

	h2_futex_wake(&test_lock,100);

	for (i = 0; i < (1<<16); i++) {
		asm volatile ("nop");
	}

	h2_futex_wake(&test_lock,NUM_GOOD_THREADS);

	for (i = 0; i < (1<<16); i++) {
		asm volatile ("nop");
	}

	h2_futex_wake(&test_lock,1);

	for (i = 0; i < (1<<16); i++) {
		asm volatile ("nop");
	}

	for (i = 0; i < NUM_GOOD_THREADS; i++) {
		if (i == 0) {
			if (thread_info[i] != 4) FAIL("counter 0 bad");
		} else {
			if (thread_info[i] != 3) FAIL("Non-0 counter bad");
		}
	}

	info("TEST PASSED\n");
	exit(0);
}

#define MAX_SIZE (1024*1024)
unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));

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
}

