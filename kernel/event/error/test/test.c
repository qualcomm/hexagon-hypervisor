/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <max.h>

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <h2.h>

H2K_thread_context *me;

/* Define H2K_fatal_kernel_handler data in the correct place */
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

/* Declarations */
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

/* H2 may call H2K_fatal_thread for some errors.  Check to make sure we 
 * were expecting the call. 
 */
void H2K_fatal_thread(u32_t r0, u32_t r1)
{
	if (TH_expect_fatal == 0) FAIL("Unexpected fatal error");
	TH_saw_fatal = 1;
	TH_clr_ex();
	longjmp(env,1);
}

/*
 * H2 may call the registered kernel fatal handler, which we should never see
 * But we need to define it so it will link.
 */

void H2K_fatal_kernel(u32_t r0, u32_t r1)
{
	FAIL("Kernel fatal error");
}

/* 
 * Again, this is needed to link.  We don't initialize anything 
 */
void H2K_fatal_init()
{
}

/*
 * FAIL the test if we see a bad vector called 
 */
void TH_bad_vector()
{
	FAIL("Bad vector called");
}

/*
 * Check the error that was called 
 * If we switched stacks, set TH_stack_switch to 1.
 * Return via longjmp
 */
void TH_error_check(int pass, u32_t sp)
{
	if (TH_expect_fatal != 0) FAIL("Expected fatal error");
	if (pass == 0) FAIL("Initial error handler checks failed");
	if ((sp > ((u32_t)(&guest_stack[0]))) && (sp <= ((u32_t)(&guest_stack[128])))) TH_stack_switch = 1;
	longjmp(env,1);
}

/*
 * If setjmp is not a nonlocal return, call the do_error helper.
 */
void TH_call_error()
{
	if (setjmp(env) == 0) {
		TH_do_error();
	}
}

int main()
{
	h2_init(NULL);
#if __QDSP6_ARCH__ <= 3
	asm (" %0 = sgp\n" : "=r"(me));
#else
	asm (" %0 = sgp0\n" : "=r"(me));
#endif
	/* 
	 * Put ourselves in guest mode, but don't have an EVB registered.
	 * We expect this to call thread_fatal
 	 */
	TH_guestmode();
	me->gevb = 0;
	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	TH_call_error();
	if (TH_saw_fatal == 0) FAIL("Didn't call fatal thread");
	puts("a");

	/*
	 * Register guest vector base
	 * We should not see fatal thread
	 * We should not switch stacks
	 */
	me->gevb = (TH_guest_vectors);
	me->gssr = 0;
	me->gosp = (u32_t)(&guest_stack[128]);
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch) FAIL("Shouldn't have switched stacks with guest bit set");
	puts("b");

	/* In user mode, with EVB set. */
	/* We should switch stacks, but not call fatal thread */
	TH_usermode();
	me->gevb = (TH_guest_vectors);
	me->gssr = 0;
	me->gosp = (u32_t)(&guest_stack[128]);
	TH_expect_fatal = 0;
	TH_saw_fatal = 0;
	TH_stack_switch = 0;
	TH_call_error();
	if (TH_saw_fatal) FAIL("Called fatal");
	if (TH_stack_switch == 0) FAIL("Should have switched stacks with guest bit clear");
	puts("c");

	/* RSVD exception should call thread fatal */
	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_rsvd();
	if (TH_saw_fatal == 0) FAIL("RSVD didn't call fatal");
	puts("d");

	/* NMI exception should call thread fatal (?) */
	TH_expect_fatal = 1;
	TH_saw_fatal = 0;
	if (setjmp(env) == 0) H2K_handle_nmi();
	if (TH_saw_fatal == 0) FAIL("NMI didn't call fatal");
	puts("e");

	puts("TEST PASSED\n");
	return 0;
}

