/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <switch.h>
#include <setjmp.h>
#include <globals.h>
#include <trace.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void H2K_wait_forever();

static jmp_buf env;

static u32_t TH_saw_wait = 0;
static u32_t TH_saw_cont = 0;

H2K_thread_context a,b;
H2K_thread_context *TH_to;
H2K_thread_context *TH_from;

void TH_wait_check()
{
	TH_saw_wait = 1;
	longjmp(env,1);
}

void TH_cont_check()
{
	TH_saw_cont = 1;
	longjmp(env,1);
}

static void modify_goto_wait()
{
	u32_t *code_snippet_address;
	__asm__ __volatile__
		(
		 " memw(r29 + #0) = r31 \n"
		 " call 1f \n"
		 " r31.h = #hi(TH_wait_check) \n"
		 " r31.l = #lo(TH_wait_check) \n"
		 " jumpr r31 \n"
		 "1: \n"
		 " %0 = r31 \n"
		 " r31 = memw(r29 +#0) \n"
		 : "=r"(code_snippet_address) : : "r31");
	memcpy(H2K_wait_forever,code_snippet_address,3*sizeof(u32_t));
}

void TH_do_switch()
{
	if (setjmp(env) == 0) {
		BKL_LOCK();
		H2K_switch(TH_from, TH_to);
	} else {
		/* longjump return */
	}
}

void TH_check_waitmode_setup()
{
}

int main()
{
	unsigned int scratch;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	__asm__ __volatile(" %0 = syscfg; %0 = clrbit(%0,#4); syscfg = %0 " : "=r"(scratch));
	H2K_trace_init();
	modify_goto_wait();
	a.continuation = TH_cont_check;
	b.continuation = TH_cont_check;
	TH_saw_wait = 0;
	TH_to = NULL;
	TH_from = NULL;
	TH_do_switch();
	if (TH_saw_wait == 0) FAIL("Did not go to wait");
	if (TH_saw_cont != 0) FAIL("went to continuation");
	TH_check_waitmode_setup();

	TH_from = &a;
	TH_to = NULL;
	TH_saw_wait = 0;
	TH_do_switch();
	if (TH_saw_wait == 0) FAIL("Did not go to wait");
	if (TH_saw_cont != 0) FAIL("went to continuation");
	TH_check_waitmode_setup();

	TH_from = NULL;
	TH_to = &a;
	TH_saw_wait = 0;
	TH_do_switch();
	if (TH_saw_wait != 0) FAIL("went to wait");
	if (TH_saw_cont == 0) FAIL("did not go to continuation");

	TH_from = &b;
	TH_to = &a;
	TH_saw_wait = 0;
	TH_do_switch();
	if (TH_saw_wait != 0) FAIL("went to wait");
	if (TH_saw_cont == 0) FAIL("did not go to continuation");
	printf("trace index=%d\n",H2K_kg.trace_info_index);
	puts("TEST PASSED\n");
	return 0;
}

