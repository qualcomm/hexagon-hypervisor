/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <h2.h>
//#include <globals.h>
#include <stdarg.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define ITERS 100

typedef unsigned int u32_t;
typedef int s32_t;

typedef unsigned long long int H2K_thread_context;

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

u32_t *TH_word;
u32_t TH_n_traps;

s32_t return_0() { TH_n_traps++; return 0; }
s32_t return_1() { TH_n_traps++; return 1; }
s32_t return_n1() { TH_n_traps++; return -1; }
s32_t bump_up_return_0() { TH_n_traps++; (*TH_word)++; return 0; }
s32_t bump_down_return_0() { TH_n_traps++; (*TH_word)--; return 0; }
s32_t bump_up_return_n1() { TH_n_traps++; (*TH_word)++; return -1; }
s32_t bump_down_return_n1() { TH_n_traps++; (*TH_word)--; return -1; }

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

void H2K_futex_cancel(H2K_thread_context *dst)
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

h2_sem_t sem;

int main()
{

	h2_sem_init_val(&sem,1);
	TH_word = (void *)(&sem);

	TH_setup_futex_wait(2,return_n1,return_0);
	/* This actually shouldn't trap... we could count traps and check... */
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");
	if (TH_n_traps != 0) FAIL("Called a trap unnecessarily");

	/* Bump the semaphore during the first trap and return failure.  Second
	 * trap returns success */
	TH_setup_futex_wait(2,bump_up_return_n1,return_0);
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");
	if (TH_n_traps == 0) FAIL("Didn't trap");
	if (TH_n_traps > 2) FAIL("Trapped too often");

	/* First trap returns success, but doesn't bump sem.  We should retrap.
	 * Second trap returns failure, but bumps sem 
	 * Second trap returns success 
	 */
	TH_setup_futex_wait(3,return_0,bump_up_return_n1,return_0);
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");
	if (TH_n_traps < 1) FAIL("Didn't trap");
	if (TH_n_traps > 3) FAIL("Trapped too often");

	puts("TEST PASSED\n");
	return 0;
}

