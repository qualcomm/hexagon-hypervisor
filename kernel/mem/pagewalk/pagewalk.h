/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEWALK_H
#define H2K_PAGEWALK_H 1

#include <c_std.h>
#include <tlbfmt.h>
#include <context.h>
#include <physread.h>
#include <translate.h>
#include <globals.h>
#include <h2_common_pagewalk.h>

/* This is rather complex */
/* For L1 page, look up PTB | ((va >> 22) << 2) */
/* For L2 page, we need bits (12+2*size) .. 22. */
/* We insert these (10-2*size) bits into PA starting at bit 2 */ 
static inline H2K_pte_t H2K_mem_pagewalk_l2(u32_t va, u32_t l2addr, u32_t tablesize, u32_t pagesize, H2K_vmblock_t *vmblock)
{
	H2K_pte_t pte;
	H2K_translation_t phys_translation;

	l2addr = Q6_R_insert_RP(l2addr,(va >> (12+(2*pagesize))),Q6_P_combine_RR(tablesize*2,2));
	if ((l2addr & 0xff000000) == 0xff000000) { // FIXME: remove this once guests no longer in monitor space
		l2addr -= H2K_gp->phys_offset;
	} else if (vmblock != NULL) {
		phys_translation =	H2K_vm_translate(l2addr, vmblock);
		if (!phys_translation.valid) {
			pte.raw = 0;
			return pte;
		}
		l2addr = phys_translation.addr;
	}
	pte.raw = H2K_mem_physread_word(l2addr);
	/* Ignore L2 page size field & overwrite */
	pte.s = pagesize;
	return pte;
}

H2K_pte_t H2K_mem_pagewalk_l1(u32_t va, u32_t baseaddr, H2K_vmblock_t *vmblock) IN_SECTION(".text.mem.pagewalk");

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

H2K_mem_tlbfmt_t H2K_mem_get_pagetable(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

static inline H2K_translation_t H2K_mem_translate_pagetable(H2K_pte_t entry, u32_t va) {

	u32_t size = PAGE_SIZE << (entry.s * 2);
	H2K_translation_t ret;

	ret.raw = 0;
	ret.addr = (va & (size - 1)) | (entry.ppn << PAGE_BITS);
	ret.size = entry.s;
	ret.cccc = entry.ccc;
	ret.xwru = (entry.xwr << 1) | entry.u;
	ret.valid = 1;

	return ret;
}

#endif
