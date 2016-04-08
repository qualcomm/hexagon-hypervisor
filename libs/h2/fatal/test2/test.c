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
	//h2_config_fatal_hook((unsigned int)PASS,(unsigned int)"PASS\n");
	h2_fatal_crash(H2_FATAL_STMODE);
	printf("OK 1\n");
	h2_fatal_crash(H2_FATAL_CLEAN);
	printf("OK 2\n");
	puts("PASS");
	h2_fatal_crash(H2_FATAL_OFF);
	FAIL("I mean, FAIL!\n");
	return 0;
}

