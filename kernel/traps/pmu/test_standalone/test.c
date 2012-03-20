/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <config.h>
#include <fatal.h>
#include <globals.h>
#include <pmu.h>
#include <hw.h>
#include <vm.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
H2K_thread_context *bptr;

#define CPUS 2
struct {
	H2K_vmblock_t vm;
	H2K_thread_context contexts[CPUS];
	u32_t pend;
	u32_t en;
	u32_t masks[CPUS];
	u32_t *maskptrs[CPUS];
} TH_vm IN_SECTION(".data.init.boot");

void TH_init_vm()
{
	memset(&TH_vm.contexts[0],0,sizeof(H2K_thread_context));
	TH_vm.vm.contexts = &TH_vm.contexts[0];
	TH_vm.vm.max_cpus = CPUS;
	TH_vm.contexts[0].id.raw = 0;
	TH_vm.contexts[0].id.vmidx = 2;
	H2K_kg.vmblocks[2] = &TH_vm.vm;
	bptr = &TH_vm.contexts[0];
}

void TH_set_dead()
{
	H2K_set_pmuevtcfg(0xdead);
	H2K_set_pmucnt0(0xdead);
	H2K_set_pmucnt1(0xdead);
	H2K_set_pmucnt2(0xdead);
	H2K_set_pmucnt3(0xdead);
}

void TH_check_dead()
{
	if (H2K_get_pmuevtcfg() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt0() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt1() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt2() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt3() != 0xdead) FAIL("unexpected changed value");
}

#define CONST_pmuevtcfg 0xffffffff
#define CONST_pmucnt0 0
#define CONST_pmucnt1 1
#define CONST_pmucnt2 2
#define CONST_pmucnt3 3

int main()
{
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	TH_init_vm();
	//H2K_thread_init();
	//H2K_readylist_init();
	//H2K_runlist_init();
	//H2K_lowprio_init();

#define TEST(X) \
	TH_set_dead(); H2K_set_##X(0xbabe); \
	if (H2K_trap_pmuconfig(2,NULL,CONST_##X, 0, &a) != 0xbabe) FAIL("Unexpected value read\n"); \
	H2K_set_##X(0xdead); TH_check_dead();

	TEST(pmuevtcfg)
	TEST(pmucnt0)
	TEST(pmucnt1)
	TEST(pmucnt2)
	TEST(pmucnt3)

#undef TEST

	puts("reads done");

#define TEST(X) \
	TH_set_dead(); \
	if (H2K_trap_pmuconfig(1,NULL,CONST_##X, 0xbabe, &a) != 0x0) FAIL("Unexpected return\n"); \
	if (H2K_get_##X() != 0xbabe) FAIL("Write failed"); \
	H2K_set_##X(0xdead); TH_check_dead();

	TEST(pmuevtcfg)
	TEST(pmucnt0)
	TEST(pmucnt1)
	TEST(pmucnt2)
	TEST(pmucnt3)

#undef TEST
	puts("writes done");

	if (H2K_trap_pmuconfig(1,NULL,0xdead,0xbabe,&a) != -1) FAIL("Unexpected return");
	if (H2K_trap_pmuconfig(2,NULL,0xdead,0xbabe,&a) != -1) FAIL("Unexpected return");

	H2K_set_pmucfg(0);

	if (bptr->pmu_on != 0) FAIL("PMU started on");
	printf("a\n");
	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != -1) FAIL("dead thread");
	if (bptr->pmu_on != 0) FAIL("PMU turned on");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	printf("b\n");
	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,4,0xbabe,&a) != -1) FAIL("dead thread");
	if (bptr->pmu_on != 0) FAIL("Changed dead thread");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	bptr->status = H2K_STATUS_BLOCKED;

	printf("c\n");
	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU turned on");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	printf("d\n");
	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,4,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 1) {
		printf("bptr->pmu_on == 0x%08x\n",bptr->pmu_on);
		FAIL(">Invalid value written");
	}
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("Unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU didn't turn off");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	bptr->status = H2K_STATUS_RUNNING;

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU turned on");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,4,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 1) FAIL("Invalid value written");
	if (H2K_get_pmucfg() != 1) FAIL("didn't change pmucfg");

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU didn't turn off");
	if (H2K_get_pmucfg() != 0) FAIL("didn't change pmucfg");

	bptr->status = H2K_STATUS_RUNNING;
	bptr->hthread = 1;

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU turned on");
	if (H2K_get_pmucfg() != 0) FAIL("Changed pmucfg");

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,4,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 1) FAIL("Invalid value written");
	if (H2K_get_pmucfg() != 2) FAIL("didn't change pmucfg");

	if (H2K_trap_pmuconfig(0,(void *)bptr->id.raw,0,0xbabe,&a) != 0) FAIL("unexpected return");
	if (bptr->pmu_on != 0) FAIL("PMU didn't turn off");
	if (H2K_get_pmucfg() != 0) FAIL("didn't change pmucfg");

	puts("setthread passed");

	puts("TEST PASSED\n");
	return 0;
}

