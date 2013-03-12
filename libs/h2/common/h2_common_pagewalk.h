/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_PAGEWALK_H
#define H2_COMMON_PAGEWALK_H 1

typedef union {
	u32_t raw;
	struct {
		u32_t s:3;
		u32_t rsvd:1;
		u32_t t:1;
		u32_t u:1;
		u32_t ccc:3;
		u32_t xwr:3;
		u32_t ppn:20;
	};
} H2K_pte_t;

#endif
