/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_fastint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

void h2_deregister_fastint(int intno)
{
	h2_register_fastint(intno, NULL);
}

