/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEWALK_H
#define H2K_PAGEWALK_H 1

typedef union {
	u32_t raw;
	struct {
		u32_t ppd:24;
		u32_t cccc:4;
		u32_t xwru:4;
	};
} H2K_pte_t;

H2K_mem_tlbfmt_t H2K_mem_translate_pagewalk(u32_t badva, H2K_thread_context *me);

#endif
