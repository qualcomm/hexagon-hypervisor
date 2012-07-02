/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asid_types.h>
#include <linear.h>
#include <pagewalk.h>

H2K_translation_t H2K_translate(u32_t addr, H2K_vmblock_t *vmblock) {

	H2K_linear_fmt_t le;
	H2K_pte_t pte;

	switch (vmblock->pmap_type) {

	case H2K_ASID_TRANS_TYPE_LINEAR:
		le = H2K_mem_lookup_linear(addr, vmblock->pmap, vmblock);
		if (le.raw == 0) return (H2K_translation_t)0ULL; // fail
		return H2K_mem_translate_linear(le, addr);

	case H2K_ASID_TRANS_TYPE_TABLE:
		pte = H2K_mem_pagewalk_l1(addr, vmblock->pmap, vmblock);
		if (pte.raw == 0) return (H2K_translation_t)0ULL; // fail
		return H2K_mem_translate_pagetable(pte, addr);

		/* Should never happen.  Be happy, gcc */
	default:
		return (H2K_translation_t)0ULL;
	}
}

