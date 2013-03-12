/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_VMBLOCK_H
#define H2_COMMON_VMBLOCK_H 1

#include <c_std.h>

typedef union {
	struct {
		u32_t size:4;
		u32_t cccc:4;
		u32_t xwru:4;
		u32_t pages:20;
	};
	u32_t raw;
} H2K_offset_t;

typedef union {
	u32_t raw;
	struct {
		u32_t cpuidx:16;
		u32_t physint:16;
	};
} H2K_physint_config_t;

#endif
