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
#include <vmevent.h>
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
	FAIL("Saw fatal");
}

H2K_kg_t H2K_kg;

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	a.ssr = 0;
	a.gevb = (void *)0x1000;
	a.elr = 0xabcd0abc;
	a.r29 = 0x290abcd0;
	a.gosp = 0xfabcdef0;
	a.gelr = a.gbadva = a.gssr = 0xdeadbeef;
	H2K_gregs_restore(&a);
	H2K_vm_event(0xcafebabe,0x1234,0x20,&a);
	H2K_gregs_save(&a);
	if (a.gssr != 0x80001234) FAIL("GSSR not set");
	if (a.gbadva != 0xcafebabe) FAIL("GBADVA not set");
	if (a.gelr != 0xabcd0abc) FAIL("GELR not exception point");
	if (a.gosp != 0x290abcd0) FAIL("GOSP not swapped");
	if (a.r29  != 0xfabcdef0) FAIL("GOSP not swapped");
	if (a.elr  != 0x1020) FAIL("ELR not gevb+offset");
	if ((a.ssr  & (1<<SSR_GUEST_BIT)) == 0) FAIL("Did not set guest mode bit");

	a.ssr = 1<<SSR_GUEST_BIT;
	a.gevb = (void *)0x1000;
	a.elr = 0xabcd0abc;
	a.r29 = 0x290abcd0;
	a.gosp = 0xfabcdef0;
	a.gelr = a.gbadva = a.gssr = 0xdeadbeef;
	H2K_gregs_restore(&a);
	H2K_vm_event(0xcafebabe,0x1234,0x20,&a);
	H2K_gregs_save(&a);
	if (a.gssr != 0x00001234) FAIL("GSSR not set");
	if (a.gbadva != 0xcafebabe) FAIL("GBADVA not set");
	if (a.gelr != 0xabcd0abc) FAIL("GELR not exception point");
	if (a.gosp != 0xfabcdef0) FAIL("GOSP swapped?");
	if (a.r29  != 0x290abcd0) FAIL("GOSP swapped?");
	if (a.elr  != 0x1020) FAIL("ELR not gevb+offset");
	if ((a.ssr  & (1<<SSR_GUEST_BIT)) == 0) FAIL("Did not keep guest mode bit");

	puts("TEST PASSED");
	return 0;
}

