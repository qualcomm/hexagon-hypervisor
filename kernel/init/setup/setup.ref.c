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

void H2K_interrupt_restore();

u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

H2K_vmblock_t *bootvm;

H2K_offset_t boot_offset = {{
		.size = BOOT_TLB_PGSIZE, // same as kernel
		.cccc = L1WB_L2C,
		.xwru = URWX,
		.pages = 0
	}};

#define DEFAULT_HEAP_SIZE 0x4000000 /* 64MB */
#define DEFAULT_STACK_SIZE 0x100000 /* 1MB */

void H2K_init_setup_bootvm(u32_t phys_offset)
{
	u32_t vm;
	BKL_UNLOCK();  // because config locks

	vm = H2K_trap_config(CONFIG_VMBLOCK_INIT, 0, SET_CPUS_INTS,
									MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT, NULL);
	bootvm = H2K_gp->vmblocks[vm];

	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PMAP_TYPE, boot_offset.raw, H2K_ASID_TRANS_TYPE_OFFSET, NULL);
	/* FIXME: Need page tables for boot VM guest->phys so that we don't need to
		 allow access to all of memory in order to be able to load at any
		 address */
	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_FENCES, 0x0, (0xffffffff >> BOOT_TLB_PGBITS) << BOOT_TLB_PGBITS, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PRIO_TRAPMASK, 0, 0xffffffff, NULL);
}

#define DEVICE_PAGE_OFFSET(ADDR) (ADDR & ((0x1 << (H2K_KERNEL_ADDRBITS + (2 * DEVICE_PAGE_SIZE))) - 1))

IN_SECTION(".text.init.setup") void H2K_init_setup(u32_t phys_offset, u32_t ssbase, u32_t last_tlb_index) {
	/* FIXME: The allocator heap can just go at the end of data once boot VM is
		 moved out of monitor space */
	void *heap_top;
	void *stack_base;
	u32_t alloc_heap_size;
	u32_t devpage_offset = DEVICE_PAGE_OFFSET(ssbase);

	heap_top = (void *)((((u32_t)&HEAP_SIZE == 0 ? DEFAULT_HEAP_SIZE : (u32_t)&HEAP_SIZE) + (u32_t)&end + 15) & -16);
	stack_base = (void *)((((u32_t)&STACK_SIZE == 0 ? DEFAULT_STACK_SIZE : (u32_t)&STACK_SIZE) +(u32_t)heap_top) & -16);
	alloc_heap_size = ((u32_t)&H2K_ALLOC_HEAP_SIZE == 0 ? DEFAULT_ALLOC_HEAP_SIZE : (u32_t)&H2K_ALLOC_HEAP_SIZE);

	H2K_kg_init(phys_offset, devpage_offset, last_tlb_index);		/* Kernel Globals first! */
	H2K_trace_init();
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_futex_init();
	H2K_intconfig_init(ssbase);
	H2K_thread_init();
	H2K_asid_table_init();
	H2K_timer_init(devpage_offset);
#ifdef HAVE_EXTENSIONS
	H2K_hvx_init(devpage_offset);
#endif
	H2K_mem_alloc_init((H2K_mem_alloc_tag_t *)((((u32_t)stack_base + 31) / 32) * 32), alloc_heap_size);
	H2K_init_setup_bootvm(phys_offset);
}

IN_SECTION(".text.init.boot") void H2K_thread_boot(u32_t phys_offset, u32_t boot_offset, u32_t ssbase, u32_t last_tlb_index)
{
	s32_t asid;

	H2K_init_setup(phys_offset, ssbase, last_tlb_index);

	/* allocate first thread */
	H2K_thread_context *boot = bootvm->free_threads;
	bootvm->free_threads = boot->next;
	bootvm->num_cpus = 1;

	boot->base_prio = boot->prio = bootvm->bestprio;
	boot->gpugp = BOOT_THREAD_GPUGP;
	boot->usr = BOOT_THREAD_USR;
	boot->ssr = (BOOT_THREAD_SSR);
	boot->elr = ((u32_t)(__bootvm_entry) - boot_offset);
	boot->r0100 = 0;
	boot->ccr = BOOT_THREAD_CCR;
	boot->trapmask = bootvm->trapmask;
	boot->continuation = H2K_interrupt_restore;
	boot->vmstatus = 0x0;
	asid = H2K_asid_table_inc((u32_t)bootvm->pmap, bootvm->pmap_type, H2K_ASID_TLB_INVALIDATE_FALSE, NULL);
	boot->ssr_asid = asid;
	BKL_LOCK();
	H2K_runlist_push(boot);
	H2K_init_complete = 1;
	H2K_mutex_unlock_tlb();
	H2K_switch(NULL,boot);
}

