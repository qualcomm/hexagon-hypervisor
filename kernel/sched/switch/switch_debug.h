/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  This file is generated from the documentation!  Do not hand edit!  */
#include <assert.h>
static void H2K_switch_debug(H2K_thread_context * from,H2K_thread_context * to) {
	H2K_thread_context * arg0 = from;
	H2K_thread_context * arg1 = to;
	ASSERT(kernel_locked())
	H2K_switch(from,to);
}

