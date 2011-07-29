/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>

main()
{
	unsigned int i,j;

	for(i = 0; i < 10*1000; i++)
	{
		asm volatile ("nop");
	}

	printf("Goodbye!\n");
}
