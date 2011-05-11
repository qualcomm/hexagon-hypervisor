/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEWALK_H
#define H2K_PAGEWALK_H 1

#include <c_std.h>
#include <tlbfmt.h>
#include <context.h>

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

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

H2K_mem_tlbfmt_t H2K_mem_translate_pagetable(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

#endif
