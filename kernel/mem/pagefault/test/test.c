/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <pagefault.h>
#include <setjmp.h>
#include <hw.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

u32_t TH_saw_fatal;

jmp_buf env;

void H2K_fatal_thread()
{
	TH_saw_fatal = 1;
	longjmp(env,1);
}

void H2K_fatal_kernel(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread) {}

H2K_kg_t H2K_kg;

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	TH_saw_fatal = 0;
	a.gevb = NULL;
	if (setjmp(env) == 0) {
		H2K_mem_pagefault(0x12345678,&a);
	}
	if (TH_saw_fatal == 0) FAIL("Didn't call thread fatal on NULL gevb");
	a.elr = 0xbadf00d8;
	a.gssr = a.gosp = a.gbadva = a.gelr = 0xdeadbeef;
	a.ssr = 1<<SSR_GUEST_BIT;
	a.r29 = 0xcafebabe;
	a.gevb = main;
	H2K_gregs_restore(&a);
	H2K_set_elr(a.elr);
	H2K_mem_pagefault(0x12345678,&a);
	H2K_gregs_save(&a);
	a.elr = H2K_get_elr();
	if (a.gssr != 0x00000022) FAIL("gssr not set");
	if (a.gosp != 0xdeadbeef) FAIL("gosp clobbered");
	if (a.gbadva != 0x12345678) FAIL("gbadva not set");
	if (a.gelr != 0xbadf00d8) FAIL("gelr not set");
	puts("TEST PASSED");
	return 0;
}

