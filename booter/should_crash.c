/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <string.h>

int main()
{
	char *p = (char *)0x01000000;
	volatile unsigned int x;

	p++;
	x = *((unsigned int *)p);
	printf("Load from 0x01000001 successful");
	return 0;
}
