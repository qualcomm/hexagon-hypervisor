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

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

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
s32_t H2K_trap_config() { return 30; }

s32_t H2K_fatal_thread() { return -1; }
s32_t H2K_fatal_kernel() { return -2; }

s32_t call_trap0(u32_t trapnum, H2K_thread_context *context);

s32_t testvals[] = {
	 0, 1, 2, 3, 4, 5, 6, 1, 8, 9,10, 1,12, 1, 1, 1,
	16, 1,18,19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,30, 1
};

H2K_kg_t H2K_kg;

int main() 
{
	s32_t i,ret;
	a.trapmask = 0xffffffff;
	for (i = 0; i < (sizeof(testvals)/sizeof(testvals[0])); i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if ((testvals[i] > 0) && (ret != testvals[i])) {
			printf("event %d: expected %d, got %d\n",i,testvals[i],ret);
			FAIL("Incorrect event return");
		}
	}

	asm volatile (
	" r0 = #-1 \n"
	" imask = r0 \n"
	" r0 = ssr \n"
	" r0 = clrbit(r0,#18) \n"
	" ssr = r0 \n" : : : "r0" );

	a.trapmask = 0xffff0000;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	a.trapmask = 0x0000ffff;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
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
		ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	a.trapmask = 0x0000ffff;
	for (i = 1; i < 16; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret < 0) {
			printf("event %d: expected pass, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}
	for (i = 16; i < 32; i++) {
		if (testvals[i] < 0) continue;
		ret = call_trap0(i,&a);
		if (ret >= 0) {
			printf("event %d: expected fail, got %d\n",i,ret);
			FAIL("Incorrect event return");
		}
	}

	puts("TEST PASSED\n");
	return 0;
}

