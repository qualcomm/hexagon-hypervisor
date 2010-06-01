/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <max.h>
#include <fatal.h>
#include <vmdefs.h>
#include <vmevent.h>
#include <pagefault.h>

void H2K_mem_pagefault(u32_t va, H2K_thread_context *me)
{
	if (me->gevb == 0) return H2K_fatal_thread(-3,me,0,0,me->hthread);
	H2K_vm_event(va,0x22,ERROR_GEVB_OFFSET,me);
}

