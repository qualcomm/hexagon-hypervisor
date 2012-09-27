/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <globals.h>
#include <tlbmisc.h>
#include <stlb.h>
#include <asid.h>
#include <vmcache.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
H2K_vmblock_t av;

u32_t TH_saw_fatal;

void H2K_fatal_thread()
{
	FAIL("Saw fatal");
}

H2K_kg_t H2K_kg;

int main()
{
	int i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	a.vmblock = &av;

	for (i = 0; i <= H2K_CACHECTL_BADOP+1; i++) {
		a.r00 = i;
		H2K_vmtrap_cachectl(&a);
		if (i < H2K_CACHECTL_BADOP) {
			if (a.r00 != 0) FAIL("bad return, exp. success");
		} else {
			if (a.r00 != -1) FAIL("bad return, exp. fail");
		}
	}

	puts("TEST PASSED");
	return 0;
}

