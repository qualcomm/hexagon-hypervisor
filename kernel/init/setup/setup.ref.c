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

void qdsp6_pre_main();
void H2K_interrupt_restore();

u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

struct {
	H2K_vmblock_t bootvm;
	H2K_thread_context bootcontexts[MAX_BOOT_CONTEXTS];
	u32_t pend;
	u32_t en;
	u32_t masks[MAX_BOOT_CONTEXTS];
	u32_t *maskptrs[MAX_BOOT_CONTEXTS];
} H2K_boot_vm IN_SECTION(".data.init.boot");

/* EJP: Use existing vmconfig routines? */

void H2K_init_setup_bootvm()
{
	int i;
	H2K_boot_vm.bootvm.pending = &H2K_boot_vm.pend;
	H2K_boot_vm.bootvm.enable = &H2K_boot_vm.en;
	H2K_boot_vm.bootvm.percpu_mask = (u32_t **)&H2K_boot_vm.maskptrs;
	H2K_boot_vm.bootvm.contexts = &H2K_boot_vm.bootcontexts[0];
	H2K_boot_vm.bootvm.max_cpus = 1;
	for (i = 0; i < MAX_BOOT_CONTEXTS; i++) {
		H2K_boot_vm.maskptrs[i] = &H2K_boot_vm.masks[i];
		H2K_thread_context_clear(&H2K_boot_vm.bootcontexts[i]);
		H2K_boot_vm.bootcontexts[i].id.raw = 0;
		H2K_boot_vm.bootcontexts[i].id.vmidx = 1;
		H2K_boot_vm.bootcontexts[i].id.cpuidx = i;
		H2K_boot_vm.bootcontexts[i].vmblock = &H2K_boot_vm.bootvm;
	}
	H2K_gp->vmblocks[1] = &H2K_boot_vm.bootvm;
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
	H2K_thread_context *boot = &H2K_boot_vm.bootcontexts[0];
	H2K_init_setup();
	boot->ssr = (BOOT_THREAD_SSR);
	boot->elr = ((u32_t)(qdsp6_pre_main));
	boot->continuation = H2K_interrupt_restore;
	boot->trapmask = 0xffffffff;
	boot->ccr = BOOT_THREAD_CCR;
	boot->sr = BOOT_THREAD_USR;
	boot->gpugp = BOOT_THREAD_GPUGP;
	asid = H2K_asid_table_inc((u32_t)H2K_linear_bootmap, H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE);
	boot->ssr_asid = asid;
	BKL_UNLOCK();
	BKL_LOCK();
	H2K_runlist_push(boot);
	H2K_init_complete = 1;
	H2K_switch(NULL,boot);
}

