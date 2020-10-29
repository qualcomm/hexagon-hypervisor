/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <cputime.h>
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
volatile int quit = 0;

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

#ifdef SIMULATOR_PMU_SUPPORT
static inline int myabs(int x)
{
	if (x < 0) {
		return -x; 
	} else return x;
}
#endif

void thread0(int thread)
{
	h2_pmu_setreg(H2_PMUEVTCFG,0);
	h2_pmu_setreg(H2_PMUCNT0,0);
	h2_pmu_setreg(H2_PMUCNT1,0);
	h2_pmu_setreg(H2_PMUCNT2,0);
	h2_pmu_setreg(H2_PMUCNT3,0);

	h2_pmu_setreg(H2_PMUEVTCFG,0x03030303);
	delay();
	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/a");
	if (h2_pmu_getreg(H2_PMUCNT0) != 0) FAIL("unexpected pmu value/b");
	if (h2_pmu_getreg(H2_PMUCNT1) != 0) FAIL("unexpected pmu value/c");
	if (h2_pmu_getreg(H2_PMUCNT2) != 0) FAIL("unexpected pmu value/d");
	if (h2_pmu_getreg(H2_PMUCNT3) != 0) FAIL("unexpected pmu value/e");
	puts("a.ok");

	//	h2_pmu_enable(t1id);  // REMOVED
	delay();
	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/f");
	if (h2_pmu_getreg(H2_PMUCNT0) != 0) FAIL("unexpected pmu value/g");
	if (h2_pmu_getreg(H2_PMUCNT1) != 0) FAIL("unexpected pmu value/h");
	if (h2_pmu_getreg(H2_PMUCNT2) != 0) FAIL("unexpected pmu value/i");
	if (h2_pmu_getreg(H2_PMUCNT3) != 0) FAIL("unexpected pmu value/j");
	puts("b.ok");

	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	delay();
	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/k");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(H2_PMUCNT0) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT1) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT2) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT3) - SPINS) > (SPINS/1024)) FAIL("Unexpected time");
#endif
	puts("c.ok");

	h2_pmu_setreg(H2_PMUCNT0,0);
	h2_pmu_setreg(H2_PMUCNT1,0);
	h2_pmu_setreg(H2_PMUCNT2,0);
	h2_pmu_setreg(H2_PMUCNT3,0);

	//	h2_pmu_disable(t1id);  // REMOVED
	h2_sem_up(&sem_call);
	h2_sem_down(&sem_ret);
	delay();
	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/l");
	if (h2_pmu_getreg(H2_PMUCNT0) != 0) FAIL("unexpected pmu value/m");
	if (h2_pmu_getreg(H2_PMUCNT1) != 0) FAIL("unexpected pmu value/n");
	if (h2_pmu_getreg(H2_PMUCNT2) != 0) FAIL("unexpected pmu value/o");
	if (h2_pmu_getreg(H2_PMUCNT3) != 0) FAIL("unexpected pmu value/p");
	puts("d.ok");

	h2_sem_add(&sem_call,2);
	delay();
	//	h2_pmu_enable(t1id);  // REMOVED
	h2_sem_down(&sem_ret);
	delay();

	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/q");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(H2_PMUCNT0) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT1) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT2) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT3) - (SPINS * 99 / 100)) > (SPINS/50)) 
		FAIL("Unexpected time");
#endif
	puts("e.ok");

	//	h2_pmu_enable(t0id);  // REMOVED
	h2_sem_add(&sem_call,1);
	delay();
	if (h2_pmu_getreg(H2_PMUEVTCFG) != 0x03030303) FAIL("unexpected pmu value/q");
#ifdef SIMULATOR_PMU_SUPPORT
	if (myabs(h2_pmu_getreg(H2_PMUCNT0) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT1) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT2) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
	if (myabs(h2_pmu_getreg(H2_PMUCNT3) - SPINS*2) > (SPINS/1024)) FAIL("Unexpected time");
#endif
	puts("f.ok");

	h2_sem_up(&sem_done);
	h2_thread_stop(0);
}

void thread1(int thread)
{
	while (1) {
		h2_sem_down(&sem_call);
		if (quit) {
			h2_thread_stop(0);
		}
		delay();
		h2_sem_up(&sem_ret);
	}
}

int main(int argc, char **argv)
{
	h2_sem_init_val(&sem_call,0);
	h2_sem_init_val(&sem_ret,0);
	h2_sem_init_val(&sem_done,0);

	t1id = h2_thread_create(thread1,&stack1[STACK_SIZE],0,2);
	t0id = h2_thread_create(thread0,&stack0[STACK_SIZE],0,2);

	h2_sem_down(&sem_done);
	quit = 1;
	h2_sem_up(&sem_call);
	puts("TEST PASSED\n");
	exit(0);
}
