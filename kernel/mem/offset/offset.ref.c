/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <offset.h>
#include <vmblock.h>
#include <h2_common_vmblock.h>
#include <globals.h>

H2K_translation_t H2K_offset_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	H2K_vmblock_t *vmblock;
	H2K_offset_t offset;
	offset.raw = info.ptb;
	in.pn += offset.pages;
	if (in.size > offset.size) in.size = offset.size;
	in.xwru &= offset.xwru;
	if (in.cccc > 0xF) in.cccc = offset.cccc;

	vmblock = H2K_gp->vmblocks[info.fields.vmid];
	if ((in.pn < vmblock->fence_lo) || (in.pn > vmblock->fence_hi)) {
		in.raw = 0;
		return in;
	}
	if (vmblock->guestmap.raw) {
		return H2K_translate(in,vmblock->guestmap);
	}	else {
		return in;
	}
}

