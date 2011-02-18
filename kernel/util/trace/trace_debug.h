/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  This file is generated from the documentation!  Do not hand edit!  */
#include <assert.h>
static void H2K_trace_debug(s8_t id,u8_t hwtnum,u8_t tid,u16_t pcyclelo) {
	s8_t arg0 = id;
	u8_t arg1 = hwtnum;
	u8_t arg2 = tid;
	u16_t arg3 = pcyclelo;
	assert(H2K_kg.trace_info.index < H2K_kg.trace_info.entries)
	H2K_trace(id,hwtnum,tid,pcyclelo);
}

