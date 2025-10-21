/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

int main() {
	h2_coproc_init();

	printf("TEST PASSED\n");
	return 0;
}

