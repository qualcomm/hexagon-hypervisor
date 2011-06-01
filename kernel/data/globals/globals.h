/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_GLOBALS_H
#define H2K_GLOBALS_H 1

#include <max.h>
#include <c_std.h>
#include <context.h>
#include <trace.h>
#include <futex.h>

typedef struct {
	u32_t mask_for_ipi;
	u64_t ready_valids[MAX_PRIOS/64] __attribute__((aligned(MAX_PRIOS/8)));
	union {
		u64_t lowprio_masks;
		struct {
			u32_t priomask;
			u32_t wait_mask;
		};
	};
	union {
		u64_t fastint_gpmask;
		struct {
			u32_t fastint_mask;
			u32_t fastint_gp;
		};
	};
	union {
		u64_t stacks_traptab;
		struct {
			void *traptab_addr;
			void *stacks_addr;
		};
	};
	u32_t tlb_index;
	H2K_thread_context *free_threads;
	u64_t oncpu_start[MAX_HTHREADS];
	H2K_trace_info_t trace_info;
	H2K_thread_context *runlist[MAX_HTHREADS];
	s16_t runlist_prios[(MAX_HTHREADS+7)/8*8] __attribute__((aligned(8)));
	H2K_thread_context *ready[MAX_PRIOS] __attribute__((aligned(MAX_PRIOS * sizeof(void *))));
	void *fastint_funcptrs[MAX_INTERRUPTS] __attribute__((aligned(MAX_INTERRUPTS * sizeof(void *))));
	H2K_thread_context *futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE * sizeof(void *))));
	void *inthandlers[MAX_INTERRUPTS]__attribute__((aligned(MAX_INTERRUPTS * sizeof(void *))));
} H2K_kg_t;

extern H2K_kg_t H2K_kg IN_SECTION(".data.core.globals");

register H2K_kg_t * const H2K_gp asm ("r16");

void H2K_kg_init() IN_SECTION(".text.init.globals");

#endif

