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
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	for (i = 0; i < MAX_PRIOS; i++) {
		a.base_prio = i;
		if (H2K_prio_get(0,&a) != i) FAIL("prio_get");
	}
	if (H2K_prio_set(&a,999,&a) != -1) FAIL("prio_set");
	// FIXME: probably should actually test real prio set behavior?
	puts("TEST PASSED\n");
	return 0;
}

