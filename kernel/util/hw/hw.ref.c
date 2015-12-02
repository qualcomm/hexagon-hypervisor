/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <hw.h>

void H2K_start_threads(unsigned int mask)
{
	asm volatile (" start(%0)" : :"r"(mask));
}
