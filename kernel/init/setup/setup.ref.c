/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
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

void qdsp6_pre_main();
void H2K_interrupt_restore();

u32_t H2K_init_complete IN_SECTION(".data.init.boot") = 0;

IN_SECTION(".text.init.setup") void H2K_init_setup()
{
	H2K_trace_init();
	H2K_fatal_init();
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_futex_init();
	H2K_intconfig_init();
	H2K_thread_init();
	H2K_kg_init();
	H2K_asid_table_init();
}

IN_SECTION(".text.init.boot") void H2K_thread_boot()
{
	s32_t asid;
	H2K_thread_context *boot = &H2K_boot_context;
	H2K_init_setup();
	boot->ssr = (BOOT_THREAD_SSR);
	boot->elr = ((u32_t)(qdsp6_pre_main));
	boot->continuation = H2K_interrupt_restore;
	boot->trapmask = 0xffffffff;
	boot->ccr = BOOT_THREAD_CCR;
	boot->sr = BOOT_THREAD_USR;
	asid = H2K_asid_table_inc((u32_t)H2K_linear_bootmap, H2K_ASID_TRANS_TYPE_LINEAR);
	boot->ssr_asid = asid;
	H2K_runlist_push(boot);
	H2K_init_complete = 1;
	H2K_switch(NULL,boot);
}

