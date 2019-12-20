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
struct H2K_vmblock_struct vmblock;
int main()
{
	u32_t i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	for (i = 0; i < MAX_PRIOS; i++) {
		a.base_prio = i;
		if (H2K_prio_get(0,&a) != i) FAIL("prio_get");
	}
	a.vmblock = NULL;
	H2K_prio_set(&a,0,&a);
	if (a.base_prio != 0) FAIL("prio_set_null");
	a.vmblock = &vmblock;
	for (i = 0; i < MAX_PRIOS-1; i++) {
		a.vmblock->bestprio = i+1;
		H2K_prio_set(&a,a.vmblock->bestprio,&a);
		if (a.base_prio != a.vmblock->bestprio) FAIL("prio_set_mid");
	}
	if (H2K_prio_set(&a,MAX_PRIOS,&a) != -1) FAIL("prio_set_limit");
	puts("TEST PASSED\n");
	return 0;
}

