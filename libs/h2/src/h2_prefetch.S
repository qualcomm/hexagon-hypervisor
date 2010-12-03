/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

	.text
	.global h2_set_prefetch
h2_set_prefetch:
#if __QDSP6_ARCH__ <= 3
	jumpr r31
#else
	r1 = usr
	r1 = insert(r0,#7,#15)
	{
		usr = r1
		jumpr r31
	}
#endif

