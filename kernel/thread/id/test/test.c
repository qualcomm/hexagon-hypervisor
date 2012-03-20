/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdio.h>
#include <stdlib.h>

u32_t H2K_thread_id(H2K_thread_context *x);

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;

int main() 
{
	a.id.raw = 0xaaaaaaaa;
	b.id.raw = 0xbbbbbbbb;
	c.id.raw = 0xcccccccc;
	if (H2K_thread_id(&a) != 0xaaaaaaaa) FAIL("bad value");
	if (H2K_thread_id(&b) != 0xbbbbbbbb) FAIL("bad value");
	if (H2K_thread_id(&c) != 0xcccccccc) FAIL("bad value");
	puts("TEST PASSED\n");
	return 0;
}

