/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <runlist.h>
#include <readylist.h>
#include <thread.h>
#include <futex.h>
#include <switch.h>
#include <lowprio.h>
#include <intconfig.h>
#include <fatal.h>
#include <asid.h>
#include <stlb.h>
#include <thread.h>
#include <hw.h>
#include <vm.h>
#include <id.h>
#include <config.h>
#include <timer.h>
#include <h2_common_pmap.h>
#include <boot.h>
#include <alloc.h>
#include <trace.h>
#include <symbols.h>
#include <subsystem.h>
#include <tmpmap.h>
#include <l2cache.h>
#include <tcm.h>
#include <sample.h>
#include <cfg_table.h>
#include <angel.h>
#include <log.h>

void H2K_interrupt_restore();

// u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

#ifndef BOOT_CCCC
#define BOOT_CCCC L1WB_L2C
#endif

static H2K_offset_t boot_offset = {{
		.size = BOOT_TLB_PGSIZE, // same as kernel
		.cccc = BOOT_CCCC,
		.xwru = URWX,
		.pages = 0
	}};

#define DEFAULT_HEAP_SIZE 0x4000000 /* 64MB */
#define DEFAULT_STACK_SIZE 0x100000 /* 1MB */

IN_SECTION(".text.init.setup") static H2K_vmblock_t *H2K_init_setup_bootvm()
{
	u32_t vm;
	//	BKL_UNLOCK();  // because config locks

	vm = H2K_trap_config(CONFIG_VMBLOCK_INIT, 0, SET_CPUS_INTS,
		MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT, NULL);

#if 0
	/* Make default guestmap 0, which means no further translation */
	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PMAP_TYPE, boot_offset.raw, H2K_ASID_TRANS_TYPE_OFFSET, NULL);
	/* FIXME: Need page tables for boot VM guest->phys so that we don't need to
		 allow access to all of memory in order to be able to load at any
		 address */
	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_FENCES, 0x0, 0xffffffff & BOOT_TLB_PAGE_MASK, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PRIO_TRAPMASK, 0, 0xffffffff, NULL);
#endif
	return H2K_gp->vmblocks[vm];
}

#define DEVICE_PAGE_OFFSET(ADDR) ((ADDR) & ((0x1 << (H2K_KERNEL_ADDRBITS + (2 * DEVICE_PAGE_SIZE))) - 1))

IN_SECTION(".text.init.setup") static H2K_vmblock_t *H2K_init_setup(u32_t multicore_shift, u32_t ssbase, u32_t last_tlb_index, u32_t tlb_size, u32_t core_id, u32_t core_count) {
	/* FIXME: The allocator heap can just go at the end of data once boot VM is
		 moved out of monitor space */
	void *heap_top;
	void *stack_base;
	u32_t alloc_heap_size;
	u32_t devpage_priv_offset = DEVICE_PAGE_OFFSET(ssbase + QDSP6SS_PUB_PRIV_OFFSET);
	u32_t devpage_pub_offset = DEVICE_PAGE_OFFSET(ssbase);

	heap_top = (void *)((((u32_t)&HEAP_SIZE == 0 ? DEFAULT_HEAP_SIZE : (u32_t)&HEAP_SIZE) + (u32_t)&end + 15) & (u32_t)-16);
	stack_base = (void *)((((u32_t)&STACK_SIZE == 0 ? DEFAULT_STACK_SIZE : (u32_t)&STACK_SIZE) +(u32_t)heap_top) & (u32_t)-16);
	alloc_heap_size = ((u32_t)&H2K_ALLOC_HEAP_SIZE == 0 ? DEFAULT_ALLOC_HEAP_SIZE : (u32_t)&H2K_ALLOC_HEAP_SIZE);

	H2K_kg_init(H2K_LINK_ADDR - multicore_shift - H2K_LOAD_ADDR, multicore_shift, devpage_priv_offset, last_tlb_index, tlb_size, core_id, core_count);		/* Kernel Globals first! */
	H2K_tmpmap_init();
	H2K_l2cache_init();
	H2K_tcm_copy(last_tlb_index);
	H2K_trace_init();
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_futex_init();
	H2K_intconfig_init(ssbase);
	H2K_thread_init();
	H2K_asid_table_init();

#ifdef CRASH_DEBUG
	H2K_stlb_tcmcrash_init();
#endif

	H2K_timer_init(devpage_priv_offset);
	H2K_hvx_init(devpage_pub_offset);
	H2K_hmx_init(devpage_priv_offset);
	H2K_mem_alloc_init((H2K_mem_alloc_tag_t *)((((u32_t)stack_base + 31) / 32) * 32), alloc_heap_size);
	H2K_sample_init();
	H2K_angel_init();
#ifdef H2K_LOGBUF
	H2K_log_init();
#endif
	H2K_log_string("Booting\n");
	return H2K_init_setup_bootvm();
}

IN_SECTION(".text.init.boot") void H2K_thread_boot(u32_t multicore_shift, u32_t ssbase, u32_t last_tlb_index, u32_t tlb_size, u32_t core_id, u32_t core_count)
{
	s32_t asid;
	H2K_vmblock_t *bootvm;

	bootvm = H2K_init_setup(multicore_shift, ssbase, last_tlb_index, tlb_size, core_id, core_count);

	/* allocate first thread */
	H2K_thread_context *boot = bootvm->free_threads;
	bootvm->free_threads = boot->next;
	bootvm->num_cpus = 1;
	bootvm->guestmap.raw = 0;
	bootvm->fence_lo = 0;
	bootvm->fence_hi = 0xffffffff & BOOT_TLB_PAGE_MASK;

	boot->base_prio = boot->prio = (u8_t)(bootvm->bestprio);
	boot->gpugp = BOOT_THREAD_GPUGP;
	boot->usr = BOOT_THREAD_USR;
	boot->ssr = (BOOT_THREAD_SSR);
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	boot->vwctrl = H2K_get_vwctrl();
#endif
	boot->elr = ((u32_t)(__bootvm_entry) - H2K_LINK_ADDR);
	boot->r0100 = multicore_shift;

	H2K_log("multicore_shift 0x%08x\n", multicore_shift);

	boot->ccr = BOOT_THREAD_CCR;
	boot->trapmask = bootvm->trapmask;
	boot->continuation = H2K_interrupt_restore;
	boot->vmstatus = 0x0;
	boot->tlbidxmask = (u8_t)~0;
	/* offset_pages should be 0 unless the kernel booted at some address other than its load addr, */
	/* such as when we clone for multicore */
	boot_offset.pages = multicore_shift >> PAGE_BITS;

	H2K_log("offset pages 0x%08x\n", boot_offset.pages);

	asid = H2K_asid_table_inc(boot_offset.raw, H2K_ASID_TRANS_TYPE_OFFSET, H2K_ASID_TLB_INVALIDATE_FALSE, 0, bootvm);
	boot->ssr_asid = (u8_t)asid;
	BKL_LOCK();

#ifdef HTHREADS_MASK

#if (HTHREADS_MASK >> MAX_HTHREADS)
#error "(HTHREADS_MASK >> MAX_HTHREADS) > 0."
#endif

#ifdef NUM_HTHREADS
#error "Can't define both NUM_HTHREADS and HTHREADS_MASK."
#endif

	H2K_start_threads(HTHREADS_MASK | 0x1); // thread 0 stays on
	H2K_isync();

	asm ( " %0 = modectl " :"=r"(H2K_gp->hthreads_mask));
	H2K_gp->hthreads_mask &= 0xffff;
	H2K_gp->hthreads = Q6_R_popcount_P(H2K_gp->hthreads_mask);

#else

#ifdef NUM_HTHREADS

#if (0 >= NUM_HTHREADS || NUM_HTHREADS > MAX_HTHREADS)
#error "Bad NUM_HTHREADS."
#endif
	u32_t i = 0;
	u32_t nthreads = 0;
	u32_t requested = NUM_HTHREADS;
	u32_t new_mask = 0;

	if (0x65 < H2K_gp->arch) {  // hthreads_mask in cfg_table
		H2K_gp->hthreads_mask = H2K_cfg_table(CFG_TABLE_HTHREADS_MASK);

		if (Q6_R_popcount_P(H2K_gp->hthreads_mask) < requested) {
			requested = Q6_R_popcount_P(H2K_gp->hthreads_mask;
		} else {
			while (nthreads < requested) {
				if (H2K_gp->hthreads_mask & (1 << i)) {
					new_mask |= (1 << i);
					nthreads++;
				}
				i++;
			}
			H2K_gp->hthreads_mask = new_mask;
		}
	} else {  // thread numbers are contiguous for ARCHV <= 65
		H2K_gp->hthreads_mask = (1 << H2K_gp->hthreads) - 1;
	}
	H2K_start_threads(H2K_gp->hthreads_mask);
	H2K_isync();
	asm ( " %0 = modectl " :"=r"(H2K_gp->hthreads_mask));
	H2K_gp->hthreads_mask &= 0xffff;
	H2K_gp->hthreads = Q6_R_popcount_P(H2K_gp->hthreads_mask);

#else
	/* boot VM will start the rest */
	H2K_gp->hthreads = 1;
	H2K_gp->hthreads_mask = 0x1;

#endif

#endif

#ifdef CLUSTER_SCHED
	H2K_cluster_config();
#endif
	H2K_runlist_push(boot);
	//	H2K_init_complete = 1;
	H2K_mutex_unlock_tlb();
	H2K_switch(NULL,boot);
}
