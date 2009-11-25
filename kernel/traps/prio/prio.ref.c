/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <prio.h>

u32_t BLASTK_prio_set(BLASTK_thread_context *dest, u32_t prio, BLASTK_thread_context *me)
{
	return me->prio;
}

u32_t BLASTK_prio_get(BLASTK_thread_context *me)
{
	return me->prio;
}

