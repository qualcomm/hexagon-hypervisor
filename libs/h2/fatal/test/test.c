/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int main()
{
	printf("Hello, World!\n");
	h2_fatal_crash();
	FAIL("should never get here");
	return 0;
}

