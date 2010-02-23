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
#include <stlb.h>
#include <tlbfmt.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

int main()
{
	H2K_mem_tlbfmt_t entry;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_mem_stlb_init();
#if __QDSP6__ARCH__ <= 3
	entry.ppn = 0;
	entry.ccc = 0;
	entry.xwr = 0x7;
	entry.size = 0;
	entry.valid = 1;
	entry.asid = 0;
	entry.vpn = 0;
#else
	entry.ppd = 1;
	entry.cccc = 0;
	entry.xwru = 0xf
	entry.valid = 1;
	entry.asid = 0;
	entry.vpn = 0;
#endif
	H2K_mem_stlb_invalidate_va(0,0,&a);
	H2K_mem_stlb_invalidate_asid(0);
	H2K_mem_stlb_add(0,0,entry,&a);
	if (H2K_mem_stlb_lookup(0,0,&a).raw != 0) {
		FAIL("found entry with no storage");
	}
	puts("FIXME: add stlb storage");
	puts("TEST PASSED");
	return 0;
}

