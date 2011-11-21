/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <globals.h>

/*
 * Strategy:
 * Fill a thread context with known values: "src".
 * Have a different, empty thread context: "dst".
 * Set up a call to the interrupt handler in various situations
 * -- fill registers with values from "src"
 * -- set SGP to "dst"
 * Check to make sure we got to the right place
 * Check to make sure the values are where they are supposed to be.
 */

void dotest_asm(void *context);

void FAIL(const char *str)
{
	puts(str);
	puts("FAIL");
	exit(1);
}

H2K_thread_context a;

int main() 
{
	int i;
	/* Set up KGP correctly for direct calls */
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	dotest_asm(&a);
	puts("TEST PASSED\n");
	return 0;
}

