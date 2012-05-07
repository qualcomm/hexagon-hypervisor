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
#include <hw.h>
//#include <globals.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

// #define SIMULATOR_PMU_SUPPORT 1

h2_sem_t sem_call,sem_ret,sem_done;
int t0id,t1id;

u64_t stack0[STACK_SIZE];
u64_t stack1[STACK_SIZE];

u64_t vmbuf[8192];

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

volatile int int_num;
h2_sem_t int_sem;
h2_sem_t int_done;

void gen_int(int which)
{
	int_num = INTERRUPT_T0 + which;
	h2_sem_up(&int_sem);
	h2_sem_down(&int_done);
}

void hw_gen_int(int which)
{
	int intno = int_num;
#if ARCHV >= 4
	swi(1<<intno);
#else
	swi(0x80000000UL>>intno);
#endif
	h2_sem_up(&int_done);
}

void vmmain(void *unused)
{
	counter0 = counter1 = 0;

	h2_sem_init_val(&sem0,0);
	h2_sem_init_val(&sem1,0);
	h2_sem_init_val(&int_sem,0);
	h2_sem_init_val(&int_done,0);
	h2_sem_init_val(&startsem,0);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_sem_down(&startsem);
	h2_sem_down(&startsem);
	delay();
	printf("Hello!\n");

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
	exit(0);
}

#define NUM_TOTAL_THREADS 4

#define MAX_SIZE (1024*1024)
unsigned long long int main_thread_stack[STACK_SIZE];
unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));
void spawn_vm(void *pc)
{
	unsigned int size;
	void *vmb;
	size = h2_config_vmblock_size(NUM_TOTAL_THREADS,1);
	printf("vmblock size: %d\n",size);
	if (size > MAX_SIZE) FAIL("Too much context needed\n");
	vmb = h2_config_vmblock_init(storage,SET_STORAGE,0,0);
	printf("vmb: %p\n",vmb);
	vmb = h2_config_vmblock_init(vmb,SET_PMAP_TYPE,0,0);
	h2_config_vmblock_init(vmb,SET_CPUS_INTS,NUM_TOTAL_THREADS,1);
	h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	printf("initted\n");
	h2_vmboot(pc,&main_thread_stack[STACK_SIZE-1],0,0,vmb);
	printf("vm booted\n");
}

int main() 
{
	u32_t asid;

	h2_init(NULL);

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
	spawn_vm(vmmain);
	while (1) {
		h2_sem_down(&int_sem);
		hw_gen_int(int_num);
	}
	h2_thread_stop();
	return 0;
}

