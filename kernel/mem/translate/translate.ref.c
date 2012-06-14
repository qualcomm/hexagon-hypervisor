/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asid_types.h>
#include <linear.h>
#include <pagewalk.h>

s32_t H2K_translate(u32_t addr, u32_t pmap, translation_type type, u32_t *result) {

	H2K_linear_fmt_t le;
	H2K_pte_t pte;

	switch (type) {

	case H2K_ASID_TRANS_TYPE_LINEAR:
		le = H2K_mem_lookup_linear(addr, pmap);

		if (le.raw == 0) { // fail
			return -1;
		}

		*result = H2K_mem_translate_linear(le, addr);
		return 0;

	case H2K_ASID_TRANS_TYPE_TABLE:
		pte = H2K_mem_pagewalk_l1(addr, pmap);

		if (pte.raw == 0) { // fail
			return -1;
		}

		*result = H2K_mem_translate_pagetable(pte, addr);
		return 0;

		/* Should never happen.  Be happy, gcc */
	default:
		return -2;
	}
}

