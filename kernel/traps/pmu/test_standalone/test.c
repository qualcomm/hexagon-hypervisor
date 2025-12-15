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
	a.id.vmidx = TH_vm.contexts[0].id.vmidx;  // trap_pmuctrl_threadset() requires same vm
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
	H2K_set_pmucnt4(0xdead);
	H2K_set_pmucnt5(0xdead);
	H2K_set_pmucnt6(0xdead);
	H2K_set_pmucnt7(0xdead);
}

void TH_check_dead()
{
	if (H2K_get_pmuevtcfg() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt0() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt1() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt2() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt3() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt4() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt5() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt6() != 0xdead) FAIL("unexpected changed value");
	if (H2K_get_pmucnt7() != 0xdead) FAIL("unexpected changed value");
}

#define CONST_pmuevtcfg 8
#define CONST_pmucnt0 0
#define CONST_pmucnt1 1
#define CONST_pmucnt2 2
#define CONST_pmucnt3 3
#define CONST_pmucnt4 4
#define CONST_pmucnt5 5
#define CONST_pmucnt6 6
#define CONST_pmucnt7 7

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	TH_init_vm();

#define TEST(X) \
	TH_set_dead(); H2K_set_##X(0xbabe); \
	if (H2K_trap_pmuctrl(PMUCTRL_GETREG, 0,CONST_##X, 0, &a) != 0xbabe) FAIL("Unexpected value read\n"); \
	H2K_set_##X(0xdead); TH_check_dead();

	TEST(pmuevtcfg)
	TEST(pmucnt0)
	TEST(pmucnt1)
	TEST(pmucnt2)
	TEST(pmucnt3)
	TEST(pmucnt4)
	TEST(pmucnt5)
	TEST(pmucnt6)
	TEST(pmucnt7)

#undef TEST

	puts("reads done");

#define TEST(X) \
	TH_set_dead(); \
	if (H2K_trap_pmuctrl(PMUCTRL_SETREG,0,CONST_##X, 0xbabe, &a) != 0x0) FAIL("Unexpected return "#X); \
	if (H2K_get_##X() != 0xbabe) FAIL("Write failed"); \
	H2K_set_##X(0xdead); TH_check_dead();

	TEST(pmuevtcfg)
	TEST(pmucnt0)
	TEST(pmucnt1)
	TEST(pmucnt2)
	TEST(pmucnt3)
	TEST(pmucnt4)
	TEST(pmucnt5)
	TEST(pmucnt6)
	TEST(pmucnt7)

#undef TEST
	puts("writes done");

	puts("TEST PASSED\n");
	return 0;
}
