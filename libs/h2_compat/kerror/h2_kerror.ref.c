/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_common_kerror.h>

const char *kerror_msg[] = {
	[KERROR_NONE]                = "No error",
	[KERROR_L2CACHE_INIT_ARCH]   = "L2 cache init: Architecture not found",
	[KERROR_L2CACHE_INIT_UARCH]  = "L2 cache init: Microarchitecture not found",
	[KERROR_L2CACHE_INIT_SIZE]   = "L2 cache init: Size reserved",
	[KERROR_L2CACHE_INIT_CONFIG] = "L2 cache init: Hardware config failed",
	[KERROR_HWCONFIG_L2REG_RANGE] = "L2 get/set reg: offset out of range"
};
