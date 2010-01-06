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
	if (H2K_thread_id(&a) != (u32_t)(&a)) FAIL("bad value");
	if (H2K_thread_id(&b) != (u32_t)(&b)) FAIL("bad value");
	if (H2K_thread_id(&c) != (u32_t)(&c)) FAIL("bad value");
	if (H2K_thread_id((void *)(0xdeadbeef)) != (u32_t)(0xdeadbeef)) FAIL("bad value");
	puts("TEST PASSED\n");
	return 0;
}

