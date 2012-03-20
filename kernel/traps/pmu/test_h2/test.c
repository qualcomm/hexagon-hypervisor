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

static inline int myabs(int x)
{
	if (x < 0) {
		return -x; 
	} else return x;
}

void thread0(int thread)
{
	h2_pmu_setreg(PMUEVTCFG,0);
	h2_pmu_setreg(PMUCNT0,0);
	h2_pmu_setreg(PMUCNT1,0);
	h2_pmu_setreg(PMUCNT2,0);
	h2_pmu_setreg(PMUCNT3,0);

	h2_pmu_setreg(PMUEVTCFG,0x03030303);
	delay();
	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/a");
	if (h2_pmu_getreg(PMUCNT0) != 0) FAIL("unexpected pmu value/b");
	if (h2_pmu_getreg(PMUCNT1) != 0) FAIL("unexpected pmu value/c");
	if (h2_pmu_getreg(PMUCNT2) != 0) FAIL("unexpected pmu value/d");
	if (h2_pmu_getreg(PMUCNT3) != 0) FAIL("unexpected pmu value/e");
	puts("a.ok");

	h2_pmu_enable(t1id);
	delay();
	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/f");
	if (h2_pmu_getreg(PMUCNT0) != 0) FAIL("unexpected pmu value/g");
	if (h2_pmu_getreg(PMUCNT1) != 0) FAIL("unexpected pmu value/h");
	if (h2_pmu_getreg(PMUCNT2) != 0) FAIL("unexpected pmu value/i");
	if (h2_pmu_getreg(PMUCNT3) != 0) FAIL("unexpected pmu value/j");
	puts("b.ok");

	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	delay();
	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/k");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(PMUCNT0) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT1) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT2) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT3) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
#endif
	puts("c.ok");

	h2_pmu_setreg(PMUCNT0,0);
	h2_pmu_setreg(PMUCNT1,0);
	h2_pmu_setreg(PMUCNT2,0);
	h2_pmu_setreg(PMUCNT3,0);

	h2_pmu_disable(t1id);
	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	delay();
	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/l");
	if (h2_pmu_getreg(PMUCNT0) != 0) FAIL("unexpected pmu value/m");
	if (h2_pmu_getreg(PMUCNT1) != 0) FAIL("unexpected pmu value/n");
	if (h2_pmu_getreg(PMUCNT2) != 0) FAIL("unexpected pmu value/o");
	if (h2_pmu_getreg(PMUCNT3) != 0) FAIL("unexpected pmu value/p");
	puts("d.ok");

	h2_sem_add(&sem_call,2);
	delay();
	h2_pmu_enable(t1id);
	h2_sem_down(&sem_ret);
	delay();

	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/q");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(PMUCNT0) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT1) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT2) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT3) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
#endif
	puts("e.ok");

	h2_pmu_enable(t0id);
	h2_sem_add(&sem_call,1);
	delay();
	if (h2_pmu_getreg(PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/q");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(PMUCNT0) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT1) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT2) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(PMUCNT3) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
#endif
	puts("f.ok");

	h2_sem_up(&sem_done);
	h2_thread_stop();
}

void thread1(int thread)
{
	while (1) {
		h2_sem_down(&sem_call);
		delay();
		h2_sem_up(&sem_ret);
	}
	h2_thread_stop();
}

void vmmain()
{

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

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
	vmb = h2_config_vmblock_init(storage,SET_STORAGE_IDENT,0,0);
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
	h2_sem_init_val(&sem_call,0);
	h2_sem_init_val(&sem_ret,0);
	h2_sem_init_val(&sem_done,0);

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
	h2_thread_stop();
	return 0;
}

