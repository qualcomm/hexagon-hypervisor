/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tid.h>

void BLASTK_tid_set(u32_t tid, BLASTK_thread_context *me)
{
	me->tid = tid;
}

u32_t BLASTK_tid_get(BLASTK_thread_context *me)
{
	return me->tid;
}

