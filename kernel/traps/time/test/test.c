/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

h2_sem_t sem_call,sem_ret,sem_done,sem_start;

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
	u64_t startpcycles;
	u64_t endpcycles;
	u64_t startcputime;
	u64_t endcputime;
	s64_t delta;
	H2K_thread_context *me;
	asm volatile (" r16 = %0 " : : "r"(&H2K_kg));
#if __QDSP6_ARCH__ <= 3
	asm volatile (" %0 = sgp" : "=r"(me));
#else
	asm volatile (" %0 = sgp0" : "=r"(me));
#endif
	startpcycles = H2K_pcycles_get(me);
	startcputime = H2K_cputime_get(me);
	//printf("start: %llx/%llx\n",startpcycles,startcputime);
	for (i = 0; i < 128; i++) {
		endpcycles = H2K_pcycles_get(me);
		endcputime = H2K_cputime_get(me);
		delta = (endpcycles - startpcycles) - (endcputime - startcputime);
		//printf("%llx/%llx (Delta: %lld)\n",endpcycles,endcputime,delta);
		if (delta < 0) delta = -delta;
		if (delta > 32) {
			FAIL("cputime/pcycles diverging");
		}
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
#warning Restore this check when H2K_cputime_get() hack for Linux removed
	//	if (delta > 1024 * MAX_HTHREADS) FAIL("Unexpected delta based on delay (D) ");

	h2_sem_up(&sem_done);
	h2_thread_stop();
}

void thread1(int thread)
{
	h2_sem_up(&sem_start);
	h2_sem_down(&sem_call);
	delay();
	h2_sem_up(&sem_ret);
	h2_thread_stop();
}

int vmmain()
{
	printf("VM Started\n");
	h2_sem_init_val(&sem_call,0);
	h2_sem_init_val(&sem_ret,0);
	h2_sem_init_val(&sem_done,0);
	//h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);
	h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	h2_sem_down(&sem_done);
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
	h2_sem_init_val(&sem_start,0);

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

	h2_sem_down(&sem_start);
	thread0(0);
	h2_thread_stop();
	exit(0);
}

