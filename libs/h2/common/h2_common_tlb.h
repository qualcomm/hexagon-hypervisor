/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_TLB_H
#define H2_COMMON_TLB_H 1

#include <h2_common_defs.h>

enum {
	TLBOP_TLBREAD,
	TLBOP_TLBWRITE,
	TLBOP_TLBQUERY,
	TLBOP_TLBALLOC,
	TLBOP_TLBFREE,
	TLBOP_MAX,
};

#endif
