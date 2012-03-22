/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>
//#include <globals.h>
#include <futex.h>
#include <stdarg.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define ITERS 100

/********
 * What's going on here?
 * We capture the implementations for futexes inside the kernel
 * Since the kernel library links with us, if we replace all the symbols from
 * futex with our own, none of those implementations will get linked in.
 * We create an array of function pointers that can get called when the traps happen.
 * Each time the trap happens, the index is incremented, so we can go through various
 * scenarios.
 * Each function pointer can do whatever it needs to, and returns the value that 
 * the trap will return.
 */

/* Arrays */

#define MAX_CAPTURES 16

typedef s32_t (*handler_t)(void);

handler_t TH_futex_wait_returns[MAX_CAPTURES];
u32_t TH_futex_wait_idx = 0;

handler_t TH_futex_resume_returns[MAX_CAPTURES];
u32_t TH_futex_resume_idx = 0;

handler_t TH_futex_lock_pi_returns[MAX_CAPTURES];
u32_t TH_futex_lock_pi_idx = 0;

handler_t TH_futex_unlock_pi_returns[MAX_CAPTURES];
u32_t TH_futex_unlock_pi_idx = 0;

/* Some example functions.  Arbitrary pointer TH_word used for modifying values in memory during the trap */

h2_allsignal_t *TH_allsig;
u32_t TH_n_traps;

s32_t return_0() { TH_n_traps++; return 0; }
s32_t return_1() { TH_n_traps++; return 1; }
s32_t return_n1() { TH_n_traps++; return -1; }

s32_t clear_waiting_return_n1() { TH_n_traps++; TH_allsig->waiting=0; return -1; }
s32_t set_waiting_return_n1() { TH_n_traps++; TH_allsig->waiting=0xffffffff; return -1; }
s32_t clear_waiting_return_0() { TH_n_traps++; TH_allsig->waiting=0; return 0; }
s32_t set_waiting_return_0() { TH_n_traps++; TH_allsig->waiting=0xffffffff; return 0; }

/*********************** Implementations of everything in kernel futex **********************/

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	return TH_futex_wait_returns[TH_futex_wait_idx++]();
}

s32_t H2K_futex_resume(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	return TH_futex_resume_returns[TH_futex_resume_idx++]();
}

s32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me)
{
	return TH_futex_lock_pi_returns[TH_futex_lock_pi_idx++]();
}

s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me)
{
	return TH_futex_unlock_pi_returns[TH_futex_lock_pi_idx++]();
}

void H2K_futex_init()
{
}

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void TH_setup_futex_wait(unsigned int entries, ...)
{
	va_list args; int i; handler_t tmp;
	if (entries > MAX_CAPTURES) entries = MAX_CAPTURES;
	va_start(args,entries);
	for (i = 0; i < entries; i++) {
		tmp = va_arg(args,handler_t);
		TH_futex_wait_returns[i] = tmp;
	}
	TH_n_traps = TH_futex_wait_idx = 0;
}

void TH_setup_futex_resume(unsigned int entries, ...)
{
	va_list args; int i; handler_t tmp;
	if (entries > MAX_CAPTURES) entries = MAX_CAPTURES;
	va_start(args,entries);
	for (i = 0; i < entries; i++) {
		tmp = va_arg(args,handler_t);
		TH_futex_resume_returns[i] = tmp;
	}
	TH_n_traps = TH_futex_resume_idx = 0;
}

h2_allsignal_t allsig;

int main()
{
//	h2_init(NULL);

	h2_allsignal_init(&allsig);
	TH_allsig = &allsig;
	
	h2_allsignal_signal(&allsig, 0x8000);
	TH_setup_futex_wait(2,return_n1,return_0);
	/* This actually shouldn't trap... we could count traps and check... */
	h2_allsignal_wait(&allsig, 0x8000);

	if (allsig.waiting != 0) FAIL("Should be waiting, but unblocked somehow.");
	if (TH_n_traps != 0) FAIL("Called a trap unnecessarily");
	h2_allsignal_init(&allsig);

	/* return -1 on first call, clear waiting and return 0 on second. */
	TH_setup_futex_wait(2,return_n1,clear_waiting_return_0);
	h2_allsignal_wait(&allsig, 0xffffffff);
	
	if (allsig.waiting != 0) FAIL("Should be waiting, but unblocked somehow.");
	if (TH_n_traps == 0) FAIL("Didn't trap");
	if (TH_n_traps > 2) FAIL("Trapped too often");

	h2_allsignal_init(&allsig);
	/* First trap returns success, but doesn't set signals.  We should retrap.
	 * Second trap clears waiting and returns failure,
	 * last trap returns success
	 */
	TH_setup_futex_wait(3,return_0,clear_waiting_return_n1,return_0);
	h2_allsignal_wait(&allsig, 0xffffffff);

	if (allsig.waiting != 0) FAIL("Should be waiting, but unblocked somehow.");
	if (TH_n_traps < 1) FAIL("Didn't trap");
	if (TH_n_traps > 3) FAIL("Trapped too often");

	puts("TEST PASSED\n");
	return 0;
}

