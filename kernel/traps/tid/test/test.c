/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <tid.h>
#include <hw.h>
#include <asm_offsets.h>
#include <stdlib.h>
#include <stdio.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a;

int main() 
{
	a.tid = 3;
	if (H2K_tid_get(&a) != 3) FAIL("Wrong tid read");
	a.tid = 0xf3;
	if (H2K_tid_get(&a) != 0xf3) FAIL("Wrong tid read");
	H2K_set_tid_reg(0x44);
	H2K_tid_set(0x55,&a);
	if (a.tid != 0x55) FAIL("Wrong tid write");
	if (H2K_get_tid_reg() != 0x55) FAIL("Wrong tid write (reg)");
	puts("TEST PASSED\n");
	return 0;
}

