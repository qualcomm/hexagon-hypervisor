/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <safemem.h>
#include <hw.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

u32_t H2K_safemem_check_and_lock(void *user_va, u32_t perms, pa_t *pa_ret, H2K_thread_context *me)
{
	u32_t tmp = (u32_t)user_va;
	H2K_mem_tlbfmt_t entry;
	s32_t idx;
	u32_t size;
	u32_t eperms;
	pa_t baseaddr;
	if (tmp & 3) return 0;
	H2K_mutex_lock_tlb();
#if ARCHV >= 4
	idx = H2K_mem_tlb_probe(tmp,me->ssr_asid);
#else
	idx = H2K_mem_tlb_probe(tmp,me->ssr_asid | me->ssr_guest << 5);
#endif
	if (idx < 0) goto fail;
	entry.raw = H2K_mem_tlb_read(idx);
	eperms = H2K_mem_tlbfmt_get_perms(entry);
	/* Check for user permission if we're user */
	if (!((me->ssr_guest) || (eperms & 1))) goto fail;
	/* Check for other permissions */
	if ((perms & (eperms >> 1)) != perms) goto fail;
	/* Obtain size and baseaddr */
	size = H2K_mem_tlbfmt_get_size(entry);
	baseaddr = H2K_mem_tlbfmt_get_basepa(entry);
	/* Use size and baseaddr to form physical address */
	baseaddr &= ((pa_t)-1)<<(size*2);
	tmp &= ((1<<(12+(size*2)))-1);
	baseaddr |= tmp;
	*pa_ret = baseaddr;
	return 1;
fail:
	H2K_mutex_unlock_tlb();
	return 0;
}

