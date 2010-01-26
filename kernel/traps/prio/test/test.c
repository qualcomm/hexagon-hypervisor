/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <prio.h>
#include <max.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

int main()
{
	u32_t i;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	for (i = 0; i < MAX_PRIOS; i++) {
		a.prio = i;
		if (H2K_prio_get(&a) != i) FAIL("prio_get");
	}
	if (H2K_prio_set(&a,4,&a) != -1) FAIL("prio_set");
	puts("TEST PASSED\n");
	return 0;
}

