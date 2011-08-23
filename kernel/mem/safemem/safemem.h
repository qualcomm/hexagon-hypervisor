/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SAFEMEM_H
#define H2K_SAFEMEM_H 1

#include <c_std.h>
#include <context.h>
#include <hw.h>

u32_t H2K_safemem_check_and_lock(void *user_va, u32_t perms, pa_t *pa_out, H2K_thread_context *me);

static inline void H2K_safemem_unlock() { H2K_mutex_unlock_tlb(); }

enum {
	SAFEMEM_NONE = 0,
	SAFEMEM_R,
	SAFEMEM_W,
	SAFEMEM_RW,
	SAFEMEM_X,
	SAFEMEM_RX,
	SAFEMEM_WX,
	SAFEMEM_RWX
};

#endif
