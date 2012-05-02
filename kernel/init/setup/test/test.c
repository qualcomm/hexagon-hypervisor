/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <futex.h>
#include <intconfig.h>
#include <thread.h>
#include <setjmp.h>
#include <globals.h>

jmp_buf env;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t TH_init_seen;
u32_t TH_switch_seen;

enum {
        fatal_init = 0,
        runlist_init,
        readylist_init,
        lowprio_init,
        futex_init,
        intconfig_init,
        kg_init,
        trace_init,
        timer_init,
        //thread_init,
        //asid_table_init,
        //mem_stlb_init,
	XX_LAST_HELPER
};

void H2K_traptab()
{
}

u64_t H2K_stacks;

void H2K_interrupt_restore()
{
}

H2K_thread_context a;

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, u32_t val4, H2K_thread_context *me)
{
	H2K_vmblock_t *block = ptr;
	block->free_threads = &a;
	block->contexts = &a;
	block->trapmask = ~0;
	return (u32_t)block;
}

#define HELPER_FUNC(X) void H2K_##X() { TH_init_seen |= 1<< X; }

HELPER_FUNC(fatal_init)
HELPER_FUNC(runlist_init)
HELPER_FUNC(readylist_init)
HELPER_FUNC(lowprio_init)
HELPER_FUNC(futex_init)
HELPER_FUNC(intconfig_init)
HELPER_FUNC(trace_init)
HELPER_FUNC(kg_init)
HELPER_FUNC(timer_init)
//HELPER_FUNC(thread_init)
//HELPER_FUNC(asid_table_init)
//HELPER_FUNC(mem_stlb_init)

extern H2K_vmblock_t *bootvm;

H2K_thread_context *boot;

/* We need to use a longjmp at the end, because H2K_switch is defined as
 * noreturn */
void H2K_switch(void *from, void *to)
{
	if (from != NULL) FAIL("Unexpected switch call");
	printf("from=%p to=%p context=%p\n",from,to,&bootvm->contexts[MAX_BOOT_CONTEXTS - 1]);
	if (to != &bootvm->contexts[MAX_BOOT_CONTEXTS - 1]) FAIL("switch to non-boot thread");
	boot = to;
	TH_switch_seen = 1;
	longjmp(env,1);
}

void H2K_trace(s8_t type, u8_t hwtnum, u8_t tid, u32_t pcyclelo)
{
}

H2K_kg_t H2K_kg;

void H2K_thread_boot();

int main()
{
	u32_t i;
	u32_t found_thread;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	for (i = 0; i < MAX_HTHREADS; i++) {
		H2K_gp->runlist[i] = 0;
	}
	TH_init_seen = 0;
	TH_switch_seen = 0;
	if (setjmp(env) == 0) {
		H2K_thread_boot();
	}
	if (TH_switch_seen == 0) FAIL("Did not switch to boot thread");
	for (i = 0; i < XX_LAST_HELPER; i++) {
		printf("%d\n",i);
		if (((1<<i) & TH_init_seen) == 0) FAIL("Didn't call init func");
	}
	if (boot->continuation != (H2K_interrupt_restore)) FAIL("Incorrect continuation");
	if (boot->trapmask != 0xffffffffU) FAIL("boot thread trapmask");
	found_thread = 0;
	for (i = 0; i < MAX_HTHREADS; i++) {
		if (H2K_gp->runlist[i] == boot) {
			if (H2K_gp->runlist_prios[i] != 0) FAIL("Didn't push into runlist (0)");
			found_thread = 1;
		}
	}
	if (!found_thread) FAIL("Didn't push into runlist (1)");
	puts("TEST PASSED\n");
	return 0;
}

