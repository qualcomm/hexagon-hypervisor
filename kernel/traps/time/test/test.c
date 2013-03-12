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
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

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
	H2K_thread_context *me;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
#if ARCHV <= 3
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
	delta = (endpcycles - startpcycles) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = -delta;
	printf("delta=%lld,pcycles=%lld\n",delta,64ULL*PCYCLES_PER_TCYCLE);
	if (delta > 64 * PCYCLES_PER_TCYCLE) {
		FAIL("Unexpected delta based on delay (A) ");
	}
	delta = (endcputime - startcputime) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = -delta;
	if (delta > 64 * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (B) ");

	startpcycles = H2K_pcycles_get(me);
	startcputime = H2K_cputime_get(me);
	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	endpcycles = H2K_pcycles_get(me);
	endcputime = H2K_cputime_get(me);
	delta = (endpcycles - startpcycles) - (SPINS * PCYCLES_PER_TCYCLE);
	if (delta < 0) delta = 0;
	if (delta > OVERHEAD * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (C) ");
	delta = (endcputime - startcputime);
	if (delta < 0) delta = -delta;
	if (delta > OVERHEAD * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay (D) ");
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

unsigned long long int main_thread_stack[STACK_SIZE];

void spawn_vm(void *pc)
{
	unsigned long vm;

	vm = h2_config_vmblock_init(0,SET_CPUS_INTS,NUM_TOTAL_THREADS,0);
	h2_config_vmblock_init(vm,SET_PMAP_TYPE,0,0);
	h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	printf("initted\n");
	h2_vmboot(pc,&main_thread_stack[STACK_SIZE-1],0,0,vm);
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

#if ARCHV <= 3
	tlb_entry |= 0x7ULL << 29;
#else
	tlb_entry |= 0xfULL << 28;
#endif

	H2K_mem_tlb_write(tlb_index, tlb_entry);

	spawn_vm(vmmain);

	h2_sem_down(&sem_start);
	thread0(0);
	h2_thread_stop(0);
	exit(0);
}

