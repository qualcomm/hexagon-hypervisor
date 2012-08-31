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
#include <bootmap_macros.h>

void qdsp6_pre_main();
void H2K_interrupt_restore();

u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

static char vmblock_space[VMBLOCK_SIZE(MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT)] IN_SECTION(".data.init.boot");

H2K_vmblock_t *bootvm;

H2K_offset_t boot_offset = {{
	.size = SIZE_16M,
	.cccc = L1WB_L2C,
	.xwru = URWX,
	.pages = 0
	}};

void H2K_init_setup_bootvm(u32_t phys_offset)
{
	BKL_UNLOCK();  // because config locks
	bootvm = (H2K_vmblock_t *)H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock_space, SET_STORAGE, 0, 0, NULL);
	H2K_gp->vmblocks[1] = bootvm;

	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_PMAP_TYPE, boot_offset.raw, H2K_ASID_TRANS_TYPE_OFFSET, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_FENCES, 0x0, 0xff000000, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_PRIO_TRAPMASK, 0, 0xffffffff, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_CPUS_INTS,
									MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT, NULL);
}

IN_SECTION(".text.init.setup") void H2K_init_setup(u32_t phys_offset)
{
	H2K_kg_init(phys_offset);		/* Kernel Globals first! */
	H2K_trace_init();
	H2K_fatal_init();
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_futex_init();
	H2K_intconfig_init();
	H2K_thread_init();
	H2K_asid_table_init();
	H2K_mem_stlb_init();
	H2K_timer_init();
	H2K_init_setup_bootvm(phys_offset);
}

IN_SECTION(".text.init.boot") void H2K_thread_boot(u32_t phys_offset)
{
	s32_t asid;

	H2K_init_setup(phys_offset);

	/* allocate first thread */
	H2K_thread_context *boot = bootvm->free_threads;
	bootvm->free_threads = boot->next;
	bootvm->num_cpus = 1;

	boot->base_prio = boot->prio = bootvm->bestprio;
	boot->gpugp = BOOT_THREAD_GPUGP;
	boot->sr = BOOT_THREAD_USR;
	boot->ssr = (BOOT_THREAD_SSR);
	boot->elr = ((u32_t)(qdsp6_pre_main));
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

