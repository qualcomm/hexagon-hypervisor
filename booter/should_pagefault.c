/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <string.h>

int main()
{
	char buf[1024];
	memcpy(buf,(void *)0x100,1024);
	printf("Memcpy from address 0x0100 successful");
	return 0;
}
