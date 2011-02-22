/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <hw.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <setjmp.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

s32_t ret;

jmp_buf env;

s32_t H2K_thread_id() { return 1; }
// s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me) { return 2; } -- defined in asm
// s32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me) { return 3; } -- defined in asm
s32_t H2K_thread_create() { return 4; }
s32_t H2K_thread_stop() { return 5; }
s32_t H2K_cputime_get() { return 6; }
s32_t H2K_register_fastint() { return 8; }
s32_t H2K_prio_set() { return 9; }
s32_t H2K_prio_get() { return 10; }
s32_t H2K_sched_yield() { return 12; }
s32_t H2K_pcycles_get() { return 16; }
s32_t H2K_tid_set() { return 18; }
s32_t H2K_tid_get() { return 19; }
//s32_t H2K_futex_lock_pi() { return 20; } -- defined in asm
//s32_t H2K_futex_unlock_pi() { return 21; } -- defined in asm
s32_t H2K_trap_config() { return 30; }
s32_t H2K_trap_hwconfig() { return 31; }

s32_t H2K_fatal_thread() { ret = -1; longjmp(env,1); }
s32_t H2K_fatal_kernel() { ret = -2; longjmp(env,1); }

s32_t call_trap0(u32_t trapnum, H2K_thread_context *context);
void TH_vectors();

void guest_mode();
void user_mode();

u64_t guest_stack[128] __attribute__((aligned(128*8)));

s32_t testvals[] = {
	 0, 1, 2, 3, 4, 5, 6, 1, 8, 9,10, 1,12, 1, 1, 1,
	16, 1,18,19,20,21, 1, 1, 1, 1, 1, 1, 1, 1,30,31
};

H2K_kg_t H2K_kg;

void TH_bad_vec()
{
	FAIL("Bad vector");
}

static inline void setup_guest()
{
	a.gevb = TH_vectors;
	a.gosp = (u32_t)(&guest_stack[127]);
}

u32_t TH_expected_guest_stack;
u32_t TH_saw_guest_trap;

void TH_guest_trap()
{
	/* Check if stack is in expected location */
	u32_t is_guest_stack;
	is_guest_stack = (((((u32_t)(&is_guest_stack)) ^ ((u32_t)(&guest_stack[120]))) & (-(sizeof(guest_stack)))) == 0);
	if (is_guest_stack != TH_expected_guest_stack) {
		printf("&is_guest_stack=0x%x, &guest_stack[120]=0x%x,mask=%x",&is_guest_stack,&guest_stack[120],-(sizeof(guest_stack)));
		printf("is_guest_stack=%d expected=%d\n",is_guest_stack,TH_expected_guest_stack);
		FAIL("Unexpected user/guest stack switch");
	}
	TH_saw_guest_trap = 1;
	longjmp(env,1);
}

void H2K_traptab();
char H2K_stacks;

int main() 
{
	s32_t i;
	H2K_kg.traptab_addr = H2K_traptab;
	H2K_kg.stacks_addr = &H2K_stacks;
	a.trapmask = 0xffffffff;
	a.gevb = NULL;
	for (i = 0; i < (sizeof(testvals)/sizeof(testvals[0])); i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if ((testvals[i] > 0) && (ret != testvals[i])) {
			printf("event %d: expected %d, got %d\n",i,testvals[i],ret);
			FAIL("Incorrect event return");
		}
	}

	/* Disable IE, so we think it's a fast interrupt */
	asm volatile (
	" r0 = #-1 \n"
	" imask = r0 \n"
	" r0 = ssr \n"
	" r0 = clrbit(r0,#18) \n"
	" ssr = r0 \n" : : : "r0" );

	a.trapmask = 0xffff0000;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	a.trapmask = 0x0000ffff;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	asm volatile (
	" r0 = #-1 \n"
	" imask = r0 \n"
	" r0 = ssr \n"
	" r0 = setbit(r0,#18) \n"
	" ssr = r0 \n" : : : "r0" );

	a.trapmask = 0xffff0000;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	a.trapmask = 0x0000ffff;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	if (setjmp(env) == 0) ret = call_trap0(32,&a);
	if (ret >= 0) FAIL("Trap didn't fail with >31 value");
	if (setjmp(env) == 0) ret = call_trap0(100,&a);
	if (ret >= 0) FAIL("Trap didn't fail with >31 value");
	if (setjmp(env) == 0) ret = call_trap0(128,&a);
	if (ret >= 0) FAIL("Trap didn't fail with >31 value");
	if (setjmp(env) == 0) ret = call_trap0(200,&a);
	if (ret >= 0) FAIL("Trap didn't fail with >31 value");

	puts("NULL gevb OK");

	guest_mode();
	setup_guest();
	TH_expected_guest_stack = 0;
	TH_saw_guest_trap = 0;
	a.trapmask = 0xffffffff;
	for (i = 0; i < (sizeof(testvals)/sizeof(testvals[0])); i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if ((testvals[i] > 0) && (ret != testvals[i])) {
			printf("event %d: expected %d, got %d\n",i,testvals[i],ret);
			FAIL("Incorrect event return");
		}
	}
	a.trapmask = 0xffff0000;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap == 0) FAIL("Didn't see expected guest trap");
		TH_saw_guest_trap = 0;
	}
	TH_saw_guest_trap = 0;
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap) {
			FAIL("Unexpected guest trap");
		}
	}

	a.trapmask = 0x0000ffff;
	TH_saw_guest_trap = 0;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap) FAIL("Unexpected guest trap");
	}
	TH_saw_guest_trap = 0;
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		}
		if (TH_saw_guest_trap == 0) FAIL("Didn't see expected guest trap");
		TH_saw_guest_trap = 0;
	}
	puts("Guest Mask Tests OK");

	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(32,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(100,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(128,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(200,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	user_mode();
	setup_guest();
	TH_expected_guest_stack = 1;

	a.trapmask = 0xffffffff;
	for (i = 0; i < (sizeof(testvals)/sizeof(testvals[0])); i++) {
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if ((testvals[i] > 0) && (ret != testvals[i])) {
			printf("event %d: expected %d, got %d\n",i,testvals[i],ret);
			FAIL("Incorrect event return");
		}
	}

	a.trapmask = 0xffff0001;
	for (i = 1; i < 16; i++) {
		user_mode(); setup_guest();
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap == 0) FAIL("Didn't see expected guest trap");
		TH_saw_guest_trap = 0;
	}
	TH_saw_guest_trap = 0;
	for (i = 16; i < 32; i++) {
		user_mode(); setup_guest();
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap) {
			FAIL("Unexpected guest trap");
		}
	}

	a.trapmask = 0x0000ffff;
	TH_saw_guest_trap = 0;
	for (i = 1; i < 16; i++) {
		user_mode(); setup_guest();
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		} 
		if (TH_saw_guest_trap) FAIL("Unexpected guest trap");
	}
	TH_saw_guest_trap = 0;
	for (i = 16; i < 32; i++) {
		user_mode(); setup_guest();
		if (testvals[i] < 0) continue;
		if (setjmp(env) == 0) {
			ret = call_trap0(i,&a);
		}
		if (TH_saw_guest_trap == 0) FAIL("Didn't see expected guest trap");
		TH_saw_guest_trap = 0;
	}
	puts("Guest Mask Tests OK (user)");

	user_mode(); setup_guest();
	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(32,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	user_mode(); setup_guest();
	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(100,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	user_mode(); setup_guest();
	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(128,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	user_mode(); setup_guest();
	TH_saw_guest_trap = 0;
	if (setjmp(env) == 0) ret = call_trap0(200,&a);
	if (TH_saw_guest_trap == 0) FAIL("guest Trap didn't fail with >31 value");

	puts("TEST PASSED\n");
	return 0;
}

