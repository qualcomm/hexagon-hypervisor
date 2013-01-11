/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_GLOBALS_H
#define H2K_GLOBALS_H 1

#include <max.h>
#include <c_std.h>
#include <context.h>
#include <futex.h>
#include <vm.h>
#include <timeinfo.h>

typedef	union {
	u64_t raw;
	struct {
		void *handler;
		void *param;
	};
} H2K_inthandler_t;

typedef struct {
	u64_t ready_valids[MAX_PRIOS/64] __attribute__((aligned(MAX_PRIOS/8)));
	H2K_timeinfo_t time;
	union {
		u64_t lowprio_masks;
		struct {
			u32_t priomask;
			u32_t wait_mask;
		};
	};
	union {
		u64_t fastint_gpunused;
		struct {
			u32_t fastint_gp;
			u32_t unused;
		};
	};
	union {
		u64_t stacks_traptab;
		struct {
			void *traptab_addr;
			void *stacks_addr;
		};
	};
#ifdef H2K_L2_CONTROL
	union {
		u64_t l2_intinfo;
		struct {
			u32_t *l2_int_base;
			u32_t *l2_ack_base;
		};
	};
#endif
	u32_t mask_for_ipi;
	u32_t tlb_index;
	union {
		u64_t trace_info_entries_buf;
		struct {
			u64_t *trace_info_buf;
			u32_t trace_info_entries;
		};
	};
	union {
		u64_t trace_info_max_level_index;
		struct {
			u32_t trace_info_index;
			s32_t trace_info_max_trace_level; // needed?
		};
	};
	u64_t oncpu_start[MAX_HTHREADS];
	u64_t oncpu_wait[MAX_HTHREADS];
	u64_t waitcycles[MAX_HTHREADS];
	H2K_thread_context *runlist[MAX_HTHREADS];
	s16_t runlist_prios[(MAX_HTHREADS+7)/8*8] __attribute__((aligned(8)));
	H2K_vmblock_t *vmblocks[H2K_ID_MAX_VMS];
	u32_t on_simulator;
	u32_t phys_offset;
	H2K_thread_context *ready[MAX_PRIOS] __attribute__((aligned(MAX_PRIOS * sizeof(void *))));
	H2K_thread_context *futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE * sizeof(void *))));
	H2K_inthandler_t inthandlers[MAX_INTERRUPTS] __attribute__((aligned(32)));
	u64_t trace_buf_default[DEFAULT_TRACE_ENTRIES];
} H2K_kg_t;

extern H2K_kg_t H2K_kg IN_SECTION(".data.core.globals");

#define GLOBAL_REG_STR "r28"

register H2K_kg_t * const H2K_gp asm (GLOBAL_REG_STR);

void H2K_kg_init(u32_t phys_offset) IN_SECTION(".text.init.globals");

#endif

