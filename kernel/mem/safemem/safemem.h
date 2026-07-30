/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SAFEMEM_H
#define H2K_SAFEMEM_H 1

/* Assembly-compatible permission constants */
#define SAFEMEM_R  (R)
#define SAFEMEM_RW (R|W)

#ifndef ASM
#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <tlbmisc.h>
#include <tlbfmt.h>

u32_t H2K_safemem_check_and_lock(void *user_va, u32_t perms, pa_t *pa_out, H2K_thread_context *me);

static inline void H2K_safemem_unlock() { H2K_mutex_unlock_tlb(); }

u32_t H2K_safemem_check_perms(void *user_va, u32_t perms, H2K_thread_context *me);

static inline u32_t H2K_safemem_check_perms_locked(u32_t user_va, u32_t perms, H2K_mem_tlbfmt_t *entry, H2K_thread_context *me)
{
	s32_t idx;
	u32_t eperms;
#if ARCHV >= 4
	idx = (s32_t)H2K_mem_tlb_probe(user_va, me->ssr_asid);
#else
	idx = H2K_mem_tlb_probe(user_va, me->ssr_asid | me->ssr_guest << 5);
#endif
	if (idx < 0) return 0;
	entry->raw = (u32_t)H2K_mem_tlb_read(idx);
	eperms = H2K_mem_tlbfmt_get_perms(*entry);
	/* Check for user permission if we're user */
	if (!((me->ssr_guest) || (eperms & U))) return 0;

	/* Check for other permissions */
	if ((perms & eperms) != perms) return 0;
	// Success
	return 1;
}

#endif

#endif
