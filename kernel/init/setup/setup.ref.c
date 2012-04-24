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
#include <bootmap.h>
#include <stlb.h>
#include <thread.h>
#include <hw.h>
#include <vm.h>
#include <id.h>
#include <config.h>

void qdsp6_pre_main();
void H2K_interrupt_restore();

u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

static char vmblock_space[VMBLOCK_SIZE(MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT)] IN_SECTION(".data.init.boot");

static unsigned long long int boot_stacks[MAX_BOOT_CONTEXTS][BOOT_STACK_SIZE];

H2K_vmblock_t *bootvm;

void H2K_init_setup_bootvm()
{
	BKL_UNLOCK();  // because config locks
	bootvm = (H2K_vmblock_t *)H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock_space, SET_STORAGE_IDENT, 0, 0, NULL);
	H2K_gp->vmblocks[1] = bootvm;

	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_PMAP_TYPE, (u32_t)H2K_linear_bootmap, H2K_ASID_TRANS_TYPE_LINEAR, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_PRIO_TRAPMASK, 0, 0xffffffff, NULL);
	H2K_trap_config(CONFIG_VMBLOCK_INIT, bootvm, SET_CPUS_INTS, MAX_BOOT_CONTEXTS, INTS_PER_BOOT_CONTEXT, NULL);
}

IN_SECTION(".text.init.setup") void H2K_init_setup()
{
	H2K_kg_init();		/* Kernel Globals first! */
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
	H2K_init_setup_bootvm();
}

IN_SECTION(".text.init.boot") void H2K_thread_boot()
{
	s32_t asid;

	H2K_init_setup();

	/* allocate first thread */
	H2K_thread_context *boot = bootvm->free_threads;
	bootvm->free_threads = boot->next;
	bootvm->num_cpus = 1;

	boot->r29 = (u32_t)&boot_stacks[0][BOOT_STACK_SIZE - 1];
	boot->ssr = (BOOT_THREAD_SSR);
	boot->elr = ((u32_t)(qdsp6_pre_main));
	boot->continuation = H2K_interrupt_restore;
	boot->trapmask = 0xffffffff;
	boot->ccr = BOOT_THREAD_CCR;
	boot->sr = BOOT_THREAD_USR;
	boot->gpugp = BOOT_THREAD_GPUGP;
	asid = H2K_asid_table_inc((u32_t)H2K_linear_bootmap, H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE);
	boot->ssr_asid = asid;
	BKL_LOCK();
	H2K_runlist_push(boot);
	H2K_init_complete = 1;
	H2K_switch(NULL,boot);
}

