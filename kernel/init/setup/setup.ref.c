/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <runlist.h>
#include <readylist.h>
#include <thread.h>
#include <futex.h>

void bootup_address();
void BLASTK_interrupt_restore();

void BLASTK_init_setup()
{
	BLASTK_runlist_init();
	BLASTK_readylist_init();
	BLASTK_futex_init();
}

void BLASTK_thread_boot()
{
	BLASTK_thread_context *boot = &BLASTK_boot_context;
	BLASTK_init_setup();
	BLASTK_thread_context_clear(boot);
	boot->ssrelr = (((u64_t)(BOOT_THREAD_SSR)) << 32) | ((u32_t)(bootup_address));
	boot->continuation = BLASTK_interrupt_restore;
	BLASTK_runlist_push(boot);
	BLASTK_switch(NULL,boot);
}

