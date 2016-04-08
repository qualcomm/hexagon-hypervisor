/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_VMBLOCK_H
#define H2_COMMON_VMBLOCK_H 1

#include <h2_common_c_std.h>

typedef union {
	struct {
		h2_u32_t size:4;
		h2_u32_t cccc:4;
		h2_u32_t xwru:4;
		h2_u32_t pages:20;
	};
	h2_u32_t raw;
} H2K_offset_t;

typedef union {
	h2_u32_t raw;
	struct {
		h2_u32_t cpuidx:16;
		h2_u32_t physint:16;
	};
} H2K_physint_config_t;

#endif
