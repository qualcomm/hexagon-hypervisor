/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <thread.h>
#include <asm_offsets.h>
#include <stdlib.h>
#include <stdio.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a;

int main() 
{
	int i;
	char *x;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	a.next = a.prev = &a;
	a.tid = a.prio = 3;
	a.gssr_gelr = 0x1234;
	a.r3130 = 0x4321;
	a.r0908 = 0x3333;
	a.cs1cs0 = 0x3456;
	H2K_thread_context_clear(&a);
	x = (void *)(&a);
	for (i = 0; i < CONTEXT_SIZE; i++) {
		if (x[i] != 0) FAIL("Nonzero element");
	}
	H2K_gp->free_threads = &a;
	H2K_thread_init();
	if (H2K_gp->free_threads != NULL) FAIL("H2K_thread_init");
	puts("TEST PASSED\n");
	return 0;
}

