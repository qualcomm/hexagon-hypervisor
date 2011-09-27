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
#include <globals.h>
#include <futex.h>
#include <stdarg.h>

#define SPINS (1024*1024)
#define STACK_SIZE 128

#define ITERS 100

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

u32_t *TH_word;

s32_t return_0() { return 0; }
s32_t return_1() { return 1; }
s32_t return_n1() { return -1; }
s32_t bump_up_return_0() { (*TH_word)++; return 0; }
s32_t bump_down_return_0() { (*TH_word)--; return 0; }
s32_t bump_up_return_n1() { (*TH_word)++; return -1; }
s32_t bump_down_return_n1() { (*TH_word)--; return -1; }

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
	TH_futex_wait_idx = 0;
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
	TH_futex_resume_idx = 0;
}

h2_sem_t sem;

int main()
{
	h2_init(NULL);

	h2_sem_init_val(&sem,1);
	TH_word = (void *)(&sem);

	TH_setup_futex_wait(2,return_n1,return_0);
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");

	TH_setup_futex_wait(2,bump_up_return_n1,return_0);
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");

	TH_setup_futex_wait(3,return_0,bump_up_return_n1,return_0);
	h2_sem_down(&sem);

	if (sem.val != 0) FAIL("Bad sem behavior");

	puts("TEST PASSED\n");
	return 0;
}

