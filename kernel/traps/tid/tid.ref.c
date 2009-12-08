/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tid.h>

void H2K_tid_set(u32_t tid, H2K_thread_context *me)
{
	me->tid = tid;
}

u32_t H2K_tid_get(H2K_thread_context *me)
{
	return me->tid;
}

