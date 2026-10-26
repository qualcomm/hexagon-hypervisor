/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

FUNC_START H2K_dosched_reset_stack .text.core.switch .p2align 2
	/* Reset sp before re-entering dosched so a mass VM kill cannot grow the
	 * thread_stop_withlock -> dosched -> switch -> vm_do_work_withlock chain. */
	r2 = htid
	r3 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r2,r3))
	r1 = r2
	r0 = #0
	jump H2K_dosched
FUNC_END H2K_dosched_reset_stack
