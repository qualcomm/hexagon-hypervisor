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

void qdsp6_pre_main();
void H2K_interrupt_restore();

void H2K_init_setup()
{
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_futex_init();
	H2K_intconfig_init();
	H2K_thread_init();
}

void H2K_thread_boot()
{
	H2K_thread_context *boot = &H2K_boot_context;
	H2K_init_setup();
	boot->ssrelr = (((u64_t)(BOOT_THREAD_SSR)) << 32) | ((u32_t)(qdsp6_pre_main));
	boot->continuation = H2K_interrupt_restore;
	boot->trapmask = 0xffffffff;
	H2K_runlist_push(boot);
	H2K_switch(NULL,boot);
}

