/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <string.h>

#define NOINLINE __attribute__((noinline))

int NOINLINE Hello()
{
	char *p = (char *)0x01000000;
	volatile unsigned int __attribute__ ((unused)) x;

	p++;
	x = *((unsigned int *)p);
	printf("Load from 0x01000001 successful");

	return 0;
}

int NOINLINE from()
{
	return Hello();
}

int NOINLINE Hexagon()
{
	return from();
}

int NOINLINE Hypervisor()
{
	return Hexagon();
}

int NOINLINE main()
{
	return Hypervisor();
}
