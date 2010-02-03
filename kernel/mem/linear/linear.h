/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LINEAR_H
#define H2K_LINEAR_H 1

#include <c_std.h>

u64_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me);

typedef union {
	u64_t raw;
	struct {
		u32_t low;
		u32_t high;
	};
} H2K_linear_list_t;

#endif

