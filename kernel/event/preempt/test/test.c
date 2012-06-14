/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <globals.h>

#define MAGIC_VAL 0xdeadbeefcafebabeULL

void FAIL(const char *str)
{
	puts(str);
	puts("FAIL");
	exit(1);
}

jmp_buf env;

u64_t TH_call_preempt(H2K_thread_context *a);
void TH_prepare();
void TH_pend_int();
void TH_imask(int imaskval);

H2K_thread_context a;

int TH_saw_interrupt;
int TH_intcheck_ok;

void TH_interrupt(int ok)
{
	TH_saw_interrupt = 1;
	TH_intcheck_ok = ok;
	longjmp(env,1);
}

void TH_preempt(H2K_thread_context *x)
{
	u64_t ret;
	if (setjmp(env) == 0) {
		if (x) x->r0100 = MAGIC_VAL;
		ret = TH_call_preempt(x);
		if (x && (ret != MAGIC_VAL)) FAIL("Didn't return r0100");
	}
}

int main() {
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	TH_prepare();
	TH_imask(-1);
	TH_saw_interrupt = 0;
	TH_preempt(NULL);
	if (TH_saw_interrupt != 0) FAIL("Interrupted NULL");
	
	TH_saw_interrupt = 0;
	TH_preempt(&a);
	if (TH_saw_interrupt != 0) FAIL("Interrupt shouldn't happen: No interrupt/imask");

	TH_imask(0);
	TH_saw_interrupt = 0;
	TH_preempt(NULL);
	if (TH_saw_interrupt != 0) FAIL("Interrupted NULL");

	TH_saw_interrupt = 0;
	TH_preempt(&a);
	if (TH_saw_interrupt != 0) FAIL("Interrupt shouldn't happen: No interrupt");

	TH_imask(-1);
	TH_pend_int();
	TH_saw_interrupt = 0;
	TH_preempt(NULL);
	if (TH_saw_interrupt != 0) FAIL("Interrupted NULL");

	TH_saw_interrupt = 0;
	TH_preempt(&a);
	if (TH_saw_interrupt != 0) FAIL("Interrupt shouldn't happen: imask");

	TH_imask(0);
	TH_saw_interrupt = 0;
	TH_preempt(NULL);
	if (TH_saw_interrupt != 0) FAIL("Interrupted NULL");

	TH_saw_interrupt = 0;
	TH_preempt(&a);
	if (TH_saw_interrupt != 1) FAIL("Didn't Interrupt");
	if (TH_intcheck_ok != 1) FAIL("Int checks not OK");

	puts("TEST PASSED\n");
	return 0;
}

