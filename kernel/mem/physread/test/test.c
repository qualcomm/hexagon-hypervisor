/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <max.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <physread.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t deadbeef = 0xdeadbeef;
u32_t cafebabe = 0xcafebabe;
u32_t *farptr = (u32_t *)(0x10000000);
u64_t *farptr2 = (u32_t *)(0x10000008);
u64_t deadbeefcafebabe = 0xdeadbeefcafebabeULL;

u32_t TH_mem_physread_word(u64_t pa)
{
	return H2K_mem_physread_word(pa);
}

u64_t TH_mem_physread_dword(u64_t pa)
{
	return H2K_mem_physread_dword(pa);
}

int main()
{
	u64_t pa;
	pa = (u64_t)((u32_t)(&deadbeef));
	if (TH_mem_physread_word(pa) != 0xdeadbeef) FAIL("word test 1");
	pa = (u64_t)((u32_t)(&cafebabe));
	if (TH_mem_physread_word(pa) != 0xcafebabe) FAIL("word test 2");
	*farptr = 0x12345678;
	pa = (u64_t)((u32_t)(farptr));
	if (TH_mem_physread_word(pa) != 0x12345678) FAIL("word test 3");
	pa = (u64_t)((u32_t)(&deadbeefcafebabe));
	if (TH_mem_physread_dword(pa) != 0xdeadbeefcafebabeULL) FAIL("dword test 1");
	*farptr2 = 0x123456789abcdef0ULL;
	pa = (u64_t)((u32_t)(farptr2));
	if (TH_mem_physread_dword(pa) != *farptr2) FAIL("dword test 2");
	puts("TEST PASSED");
	return 0;
}

