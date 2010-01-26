/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
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
        thread_init,
        trace_init,
	XX_LAST_HELPER
};

void H2K_interrupt_restore()
{
}

#define HELPER_FUNC(X) void H2K_##X() { TH_init_seen |= 1<< X; }

HELPER_FUNC(fatal_init)
HELPER_FUNC(runlist_init)
HELPER_FUNC(readylist_init)
HELPER_FUNC(lowprio_init)
HELPER_FUNC(futex_init)
HELPER_FUNC(intconfig_init)
HELPER_FUNC(thread_init)
HELPER_FUNC(trace_init)

/* We need to use a longjmp at the end, because H2K_switch is defined as
 * noreturn */
void H2K_switch(void *from, void *to)
{
	if (from != NULL) FAIL("Unexpected switch call");
	if (to != &H2K_boot_context) FAIL("switch to non-boot thread");
	TH_switch_seen = 1;
	longjmp(env,1);
}

H2K_thread_context H2K_boot_context;

void H2K_thread_boot();

int main()
{
	u32_t i;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->runlist[i] = 0;
	}
	H2K_gp->runlist_valids = 0;
	TH_init_seen = 0;
	TH_switch_seen = 0;
	if (setjmp(env) == 0) {
		H2K_thread_boot();
	}
	if (TH_switch_seen == 0) FAIL("Did not switch to boot thread");
	for (i = 0; i < XX_LAST_HELPER; i++) {
		if (((1<<i) & TH_init_seen) == 0) FAIL("Didn't call init func");
	}
	if (H2K_boot_context.continuation != (H2K_interrupt_restore)) FAIL("Incorrect continuation");
	if (H2K_boot_context.trapmask != 0xffffffffU) FAIL("boot thread trapmask");
	if (H2K_gp->runlist_valids != 1) FAIL("Didn't push into runlist");
	if (H2K_gp->runlist[0] != &H2K_boot_context) FAIL("Didn't push into runlist");
	puts("TEST PASSED\n");
	return 0;
}

