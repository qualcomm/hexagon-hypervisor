/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <c_std.h>
#include <pagewalk.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <hw.h>
#include <hexagon_protos.h>
#include <context.h>
#include <asid.h>
#include <pagewalk.h>

static inline H2K_translation_t H2K_pagewalk_update_translation(H2K_translation_t in, H2K_pte_t pte)
{
	u32_t size = pte.s;
	in.size = min(in.size,size);
	/* Carefully update pn since later translation might be < current size */
	in.pn &= (1<<(size*2))-1;
	in.pn |= pte.ppn & (-1<<(size*2));
	in.xwru &= (pte.xwr << 1) | pte.u;
	if (in.cccc > 0xF) in.cccc = pte.ccc;
	return in;
}

/* This is rather complex */
/* For L1 page, look up PTB | ((va >> 22) << 2) */
/* For L2 page, we need bits (12+2*size) .. 22. */
/* We insert these (10-2*size) bits into PA starting at bit 2 */ 
static inline H2K_pte_t H2K_mem_pagewalk_l2(H2K_translation_t in, u32_t l2addr, u32_t tablesize, u32_t pagesize, H2K_vmblock_t *vmblock)
{
	H2K_pte_t pte;
	H2K_translation_t tmp;
	pa_t l2_paddr = l2addr;
	l2_paddr = Q6_P_insert_PP(l2_paddr,(in.pn >> (2*pagesize)),Q6_P_combine_RR(tablesize*2,2));
	if (vmblock->guestmap.raw) {
		tmp = H2K_translate_default(l2_paddr);
		tmp = H2K_translate(tmp,vmblock->guestmap);
		if ((tmp.xwru & 2) == 0) {
			pte.raw = 0;
			return pte;
		}
		l2_paddr = Q6_P_insert_PII(l2_paddr,tmp.pn,24,12);
	}
	pte.raw = H2K_mem_physread_word(l2_paddr);
	/* Ignore L2 page size field & overwrite */
	pte.s = pagesize;
	return pte;
}

static inline H2K_pte_t H2K_mem_pagewalk_l1(H2K_translation_t in, H2K_asid_entry_t info, H2K_vmblock_t *vmblock)
{
	H2K_pte_t pte;
	u32_t size;
	u32_t baseaddr = info.ptb;
	H2K_translation_t tmp;
	/* check that effective addr is in bounds of VM */
	/* FIXME: or, translate at ptb registration time! */
	pa_t gpn = baseaddr >> PAGE_BITS;
	pa_t ppn;
	if (vmblock->guestmap.raw) {
		tmp.pn = gpn;
		tmp = H2K_translate(tmp,vmblock->guestmap);
		if ((tmp.xwru & 2) == 0) goto fail;
		ppn = tmp.pn;
	} else {
		ppn = gpn;
	}
	pte.raw = H2K_mem_physread_word((ppn << PAGE_BITS)| ((in.pn>>8) & 0xffc));
	size = pte.s;
	if (size <= 4) return H2K_mem_pagewalk_l2(in,pte.raw & -16, 5-size, size, vmblock);
	if (size == 7) goto fail;
	/* REFINE PPN, PERMS */
	return pte;
fail:
	pte.raw = 0;
	return pte;
}

H2K_translation_t H2K_pagewalk_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	H2K_pte_t pte;
	H2K_vmblock_t *vmblock = H2K_gp->vmblocks[info.fields.vmid];
	pte = H2K_mem_pagewalk_l1(in,info,vmblock);
	in = H2K_pagewalk_update_translation(in,pte);
	if (vmblock->guestmap.raw) return H2K_translate(in, vmblock->guestmap);
	else return in;
}

