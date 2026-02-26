/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <hexagon_protos.h>
#include <asid.h>
#include <varadix.h>
#include <physread.h>
#include <globals.h>

static inline u32_t H2K_varadix_entry_size(u32_t val)
{
	return Q6_R_ct0_R(val);
}

static inline H2K_translation_t H2K_varadix_update(H2K_translation_t in, H2K_translation_t new)
{
	in.pn = new.pn;
	in.xwru &= new.xwru;
	in.shared = new.shared;
	if (new.raw == 0) return new;
	if (in.weak_ccc) in.cccc = new.cccc;
	if (in.size > new.size) in.size = new.size;
	in.weak_ccc = new.weak_ccc;
	return in;
}

static inline H2K_translation_t H2K_varadix_mktrans(u32_t vpn, u32_t entry, u32_t size, u32_t unused, H2K_vmblock_t *vmblock)
{
	H2K_translation_t ret;
	u32_t maxsize = 12;
	u32_t mypn;
	ret.raw = 0;
	ret.xwru = entry >> VARADIX_XWRU_OFFSET;
	ret.cccc = entry >> VARADIX_CCCC_OFFSET;
	entry &= entry-1;	// clear least significant set bit
	/* Must preserve all low order bits in case pages get smaller (by stage2 translation or odd size) */
	mypn = Q6_R_extractu_RP(entry,Q6_P_combine_RR(VARADIX_PN_BITS-size,1+size));
	ret.pn = Q6_R_insert_RP(vpn,mypn,Q6_P_combine_RR(VARADIX_PN_BITS-size,size));
	if (size > maxsize) {
		size = maxsize;
	}
	ret.size = (u8_t)(size >> 1);
	return ret;
}

/*
 * EJP: FIXME: optimization: can keep last valid translation to sometimes skip
 * H2K_translate call during walk but, this requires more args to an already
 * pretty full signature.  Converting from recursive to iterative might help,
 * but makes complexity somewhat higher.
 */
static inline H2K_translation_t H2K_varadix_walk(
	u32_t vpn, 
	u32_t tablebase, 
	u32_t tablesize, 
	u32_t startbit, 
	H2K_vmblock_t *vmblock)
{
	H2K_translation_t tmp;
	u32_t entry;
	u32_t nextsize;
	u32_t index = Q6_R_extractu_RP(vpn,Q6_P_combine_RR(tablesize,startbit));
	u32_t tableaddr32 = Q6_R_insert_RP(tablebase,index,Q6_P_combine_RR(tablesize,0));
	pa_t tableaddr = (pa_t)tableaddr32 << 2;
	if (vmblock->guestmap.raw) {
		tmp = H2K_translate_default(tableaddr);
		tmp = H2K_translate(tmp,vmblock->guestmap);
		if ((tmp.xwru & 2) == 0) return H2K_translate_bad();
		tableaddr = Q6_P_insert_PII(tableaddr,tmp.pn,24,12);
	}
	entry = H2K_mem_physread_word(tableaddr);
	nextsize = H2K_varadix_entry_size(entry);
	if (unlikely(nextsize > startbit)) return H2K_translate_bad();
	if (nextsize == startbit) return H2K_varadix_mktrans(vpn,entry,nextsize,0,vmblock);
	return H2K_varadix_walk(vpn,entry,nextsize+1,startbit-(nextsize+1),vmblock);
}

H2K_translation_t H2K_varadix_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	u32_t vpnbits = info.fields.extra;
	u32_t startinfo = info.ptb;
	u32_t startsize = H2K_varadix_entry_size(startinfo);
	H2K_vmblock_t *vmblock = H2K_gp->vmblocks[info.fields.vmid];
	H2K_translation_t new;
	if (unlikely(in.pn >= (1<<vpnbits))) {
		new = H2K_translate_bad();
	} else if (unlikely(vpnbits == startsize)) {
		new = H2K_varadix_mktrans(in.pn,info.ptb,startsize,0,vmblock);
	} else {
		new = H2K_varadix_walk(in.pn,info.ptb,startsize+1,vpnbits-(startsize+1),vmblock);
	}
	in = H2K_varadix_update(in,new);
	if (!in.shared && vmblock->guestmap.raw) return H2K_translate(in,vmblock->guestmap);
	else return in;
}

