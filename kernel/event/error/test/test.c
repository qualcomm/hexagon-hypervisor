/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <max.h>
#include <h2.h>

H2K_thread_context *me;

void (*H2K_fatal_kernel_handler)(u32_t) IN_SECTION(".data.error.fatal");

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t TH_expect_fatal;
u32_t TH_saw_fatal;
u32_t TH_stack_switch;

jmp_buf env;

u64_t guest_stack[128];

void H2K_fatal_thread(u32_t r0, u32_t r1)
{
	if (TH_expect_fatal == 0) FAIL("Unexpected fatal error");
	TH_saw_fatal = 1;
	longjmp(env,1);
}

void H2K_fatal_kernel(u32_t r0, u32_t r1)
{
	FAIL("Kernel fatal error");
}

void H2K_fatal_init()
{
}

void TH_error_check(int pass, u32_t sp)
{
	if (TH_expect_fatal != 0) FAIL("Expected fatal error");
	if (pass == 0) FAIL("Initial error handler checks failed");
	if ((sp > ((u32_t)(&guest_stack[0]))) && (sp <= ((u32_t)(&guest_stack[128])))) TH_stack_switch = 1;
	longjmp(env,1);
}

void TH_error_handler();
void TH_do_error();

void H2K_handle_rsvd();
void H2K_handle_nmi();
void H2K_handle_trap1();

void TH_call_error()
{
	if (setjmp(env) == 0) {
		TH_do_error();
	}
}

int main()
{
	h2_init(NULL);
	asm (" %0 = sgp\n" : "=r"(me));
	me->gevb = 0;
	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	TH_call_error();
	if (TH_saw_fatal == 0) FAIL("Didn't call fatal thread");

	asm volatile (
	" r9 = ssr \n"
	" r9 = setbit(r9,#13) \n"
	" ssr = r9 \n" : : : "r9");
	me->gevb = (TH_error_handler);
	me->gssr_gosp = 0x0000000000000000ULL | ((u32_t)(&guest_stack[128]));
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch) FAIL("Shouldn't have switched stacks with guest bit set");

	asm volatile (
	" r9 = ssr \n"
	" r9 = clrbit(r9,#13) \n"
	" ssr = r9 \n" : : : "r9");
	me->gevb = (TH_error_handler);
	me->gssr_gosp = 0x0000000000000000ULL | ((u32_t)(&guest_stack[128]));
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch == 0) FAIL("Should have switched stacks with guest bit clear");

	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_rsvd();
	if (TH_saw_fatal == 0) FAIL("RSVD didn't call fatal");

	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_trap1();
	if (TH_saw_fatal == 0) FAIL("TRAP1 didn't call fatal");

	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_nmi();
	if (TH_saw_fatal == 0) FAIL("NMI didn't call fatal");

	puts("TEST PASSED\n");
	return 0;
}

