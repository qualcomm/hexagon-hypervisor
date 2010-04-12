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

void TH_error_handler();
void TH_do_error();
void TH_guest_vectors();
void TH_guestmode();
void TH_usermode();
void TH_clr_ex();

void H2K_handle_rsvd();
void H2K_handle_nmi();
void H2K_handle_trap1();

jmp_buf env;

u64_t guest_stack[128];

void H2K_fatal_thread(u32_t r0, u32_t r1)
{
	if (TH_expect_fatal == 0) FAIL("Unexpected fatal error");
	TH_saw_fatal = 1;
	TH_clr_ex();
	longjmp(env,1);
}

void H2K_fatal_kernel(u32_t r0, u32_t r1)
{
	FAIL("Kernel fatal error");
}

void H2K_fatal_init()
{
}

void TH_bad_vector()
{
	FAIL("Bad vector called");
}

void TH_error_check(int pass, u32_t sp)
{
	if (TH_expect_fatal != 0) FAIL("Expected fatal error");
	if (pass == 0) FAIL("Initial error handler checks failed");
	if ((sp > ((u32_t)(&guest_stack[0]))) && (sp <= ((u32_t)(&guest_stack[128])))) TH_stack_switch = 1;
	longjmp(env,1);
}

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
	TH_guestmode();
	me->gevb = 0;
	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	TH_call_error();
	if (TH_saw_fatal == 0) FAIL("Didn't call fatal thread");
	puts("a");

	me->gevb = (TH_guest_vectors);
	me->gssr_gosp = 0x0000000000000000ULL | ((u32_t)(&guest_stack[128]));
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch) FAIL("Shouldn't have switched stacks with guest bit set");
	puts("b");

	TH_usermode();
	me->gevb = (TH_guest_vectors);
	me->gssr_gosp = 0x0000000000000000ULL | ((u32_t)(&guest_stack[128]));
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch == 0) FAIL("Should have switched stacks with guest bit clear");
	puts("c");

	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_rsvd();
	if (TH_saw_fatal == 0) FAIL("RSVD didn't call fatal");
	puts("d");

	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_nmi();
	if (TH_saw_fatal == 0) FAIL("NMI didn't call fatal");
	puts("e");

	puts("TEST PASSED\n");
	return 0;
}

