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

void PASS(const char *msg)
{
	asm volatile (" k0unlock; tlbunlock; \n");
	if (msg == NULL) FAIL("bad arg");
	puts(msg);
	puts("OK?");
}

int main()
{
	printf("Hello, World!\n");
	//h2_config_fatal_hook((unsigned int)PASS,(unsigned int)"PASS\n");
	h2_fatal_crash(~0);
	FAIL("should never get here");
	return 0;
}

