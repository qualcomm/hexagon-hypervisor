/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_KERROR_H
#define H2_COMMON_KERROR_H 1

typedef enum {
	KERROR_NONE,
	KERROR_L2CACHE_INIT_ARCH,
	KERROR_L2CACHE_INIT_UARCH,
	KERROR_L2CACHE_INIT_SIZE,
	KERROR_L2CACHE_INIT_CONFIG,
	KERROR_HWCONFIG_L2REG_RANGE,
	KERROR_HWCONFIG_CLADEREG_RANGE,
	KERROR_MAX
} kerror_type;

#endif
