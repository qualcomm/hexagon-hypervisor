/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <hwconfig.h>

void h2_init();

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int main()
{
	u32_t tmp,i;
	h2_init();
	H2K_trap_hwconfig_hwthreads_mask(0, NULL, -1, 0, NULL);  // start all hw threads

	for (i = 0; i < 1000; i++) {
		h2_init();
	}
	asm ( " %0 = modectl " :"=r"(tmp));
	if ((tmp & 0x00fe) == 0) FAIL("Didn't start up other threads");
	asm ( " %0 = syscfg " : "=r"(tmp));
	if ((tmp & 7) != 7) FAIL("Didn't start up syscfg correctly");
	puts("TEST PASSED\n");
	return 0;
}

