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
#include <stlb.h>
#include <asid_types.h>
#include <spinlock.h>
#include <timeinfo.h>
#include <h2_common_info.h>
#include <h2_common_kerror.h>

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
		u64_t fastint_gp_ssr;
		struct {
			u32_t fastint_ssr;
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
	union {
		u64_t syscfg_stlbptr;
		struct {
			H2K_mem_stlb_asid_info_t *stlbptr;
			u32_t syscfg_val;
		};
	};
	union {
		u32_t core_rev;
		struct {
			u32_t arch:8;
			u32_t uarch:4;
			u32_t l2arr:4;
			u32_t metal:16;
		};
	};
	u32_t is_nmi_soft;

#ifdef H2K_L2_CONTROL
	union {
		u64_t l2_intinfo;
		struct {
			u32_t *l2_int_base;
			u32_t *l2_ack_base;
		};
	};
#endif
	u32_t timer_intnum;
	u32_t *hvx_clock;
	u32_t *hvx_reset;
	u32_t *hvx_power;
#if ARCHV >= 65
	u32_t *hvx_bhs_status;
	u32_t *hvx_bhs_cfg;
	u32_t *hvx_bhs_cmd;
	u32_t *hvx_cpmem_cfg;
	u32_t *hvx_cpmem_cmd;
	u32_t *hvx_cpmem_status;
#endif
	u32_t hvx_state;
	u32_t hvx_vlength;  // native vector length

	union {
		u64_t fatal_hook_and_arg;
		struct {
			u32_t fatal_hook_arg;
			u32_t fatal_hook;
		};
	};
	union {
		u64_t fatal_hook_gp_ssr;
		struct {
			u32_t fatal_hook_ssr;
			u32_t fatal_hook_gp;
		};
	};

	u32_t mask_for_ipi;
	u32_t tlb_index;
	u32_t last_tlb_index;
	H2K_spinlock_t tmpmap_lock;
	u32_t tlb_size;
	u64_t pinned_tlb_mask;
	H2K_spinlock_t asid_spinlock;
	u64_t oncpu_start[MAX_HTHREADS];
	u64_t oncpu_wait[MAX_HTHREADS];
	u64_t waitcycles[MAX_HTHREADS];
#ifdef DO_PROFILE
	u64_t prof_sample[MAX_HTHREADS];
	u32_t prof_sample_pending;
	H2K_spinlock_t prof_sample_lock;
#endif
	H2K_thread_context *runlist[MAX_HTHREADS];
	s16_t runlist_prios[(MAX_HTHREADS+7)/8*8] __attribute__((aligned(8)));
	H2K_vmblock_t *vmblocks[H2K_ID_MAX_VMS];
	u32_t phys_offset;
	u32_t build_id;
	info_boot_flags_type info_boot_flags;
	info_stlb_type info_stlb;
	kerror_type kernel_error;
	u32_t hthreads;
	u32_t hthreads_mask;
	u32_t tcm_base; // FIXME: pa_t ?
	u32_t tcm_size;
	u32_t l2size;
	u32_t l2tags;

#ifdef CRASH_DEBUG
	u64_t crash_tlb[MAX_TLB_ENTRIES];
	u32_t crash_l2vic_enabled[MAX_INTERRUPTS/32];
	u32_t crash_l2vic_pending[MAX_INTERRUPTS/32];
	H2K_thread_context crash_contexts[MAX_HTHREADS];
#endif
		
	H2K_inthandler_t inthandlers[MAX_INTERRUPTS] __attribute__((aligned(32)));
	H2K_thread_context *futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE * sizeof(void *))));
	H2K_asid_entry_t asid_table[MAX_ASIDS] __attribute__((aligned(MAX_ASIDS*sizeof(H2K_asid_entry_t))));
	H2K_thread_context *ready[MAX_PRIOS] __attribute__((aligned(MAX_PRIOS * sizeof(void *))));
} H2K_kg_t;

extern H2K_kg_t H2K_kg IN_SECTION(".data.core.globals");

#define GLOBAL_REG_STR "r28"

#ifndef __llvm__
register H2K_kg_t * const H2K_gp asm (GLOBAL_REG_STR);
#else

#define PURITY __attribute__((const))
static inline H2K_kg_t PURITY *H2K_gp_llvm()
{
	register H2K_kg_t *ret __asm__("r28");
	__asm__ (" /* Fool the compiler */ " : "=r"(ret));
	return ret;
}
#define H2K_gp H2K_gp_llvm()
#undef PURITY
#endif

void H2K_kg_init(u32_t phys_offset, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size) IN_SECTION(".text.init.globals");

#endif

