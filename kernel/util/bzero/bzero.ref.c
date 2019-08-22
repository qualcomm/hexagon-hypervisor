/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <bzero.h>

void H2K_bzero(void *dst, u32_t size)
{
	unsigned char *p = dst;
	while (size) {
		*p++ = 0;
		size--;
	}
}

