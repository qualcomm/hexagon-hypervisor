/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <runlist.h>
#include <lowprio.h>
#include <context.h>
#include <hw.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <fatal.h>
#include <stop.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("ERROR:  ");
	puts(str);
	exit(1);
}

jmp_buf env;

u32_t TH_expected_stop = 0;
u32_t TH_saw_stop = 0;

u32_t TH_expected_handler = 0;
u32_t TH_saw_handler = 0;

H2K_thread_context a;

void  __attribute__((noreturn)) TH_handler(u32_t foo)
{
	if (TH_expected_handler == 0) FAIL("Didn't expect handler");
	TH_saw_handler = 1;
	longjmp(env,1);
}

void H2K_thread_stop(u32_t status, H2K_thread_context *me)
{
	if (TH_expected_stop == 0) FAIL("Didn't expect thread stop");
	TH_saw_stop = 1;
	longjmp(env,1);
}

int main() 
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_fatal_init();
	H2K_trace_init();
	H2K_fatal_kernel_handler = TH_handler;
	TH_expected_stop = TH_saw_stop = 0;
	TH_expected_handler = TH_saw_handler = 0;

	TH_expected_stop = 1;
	if (setjmp(env) == 0) H2K_fatal_thread(-1,&a,0,1,0);
	if (TH_saw_stop == 0) FAIL("Didn't see H2K_thread_stop call");

	TH_expected_stop = TH_saw_stop = 0;
	TH_expected_handler = TH_saw_handler = 0;

	TH_expected_handler = 1;
	if (setjmp(env) == 0) H2K_fatal_kernel(-1,&a,0,1,0);
	if (TH_saw_handler == 0) FAIL("Didn't see handler call");

	H2K_fatal_init();
	puts("TEST PASSED (unless more text follows)");
	H2K_fatal_kernel(-1,&a,0,1,0);

	H2K_fatal_kernel(-1,&a,0,1,0);
	FAIL("ERROR:  Didn't terminate test");
	return 0;
}

