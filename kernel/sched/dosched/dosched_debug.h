/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  This file is generated from the documentation!  Do not hand edit!  */
#include <assert.h>
static void H2K_dosched_debug(H2K_thread_context * me,u32_t hthread) {
	H2K_thread_context * arg0 = me;
	u32_t arg1 = hthread;
	assert(kernel_locked());
	H2K_dosched(me,hthread);
}

