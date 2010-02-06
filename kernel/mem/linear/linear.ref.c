/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <linear.h>
#include <q6protos.h>

#if __QDSP6_ARCH__ <= 3

u64_t H2K_mem_convert_to_native(H2K_linear_list_t list_entry, H2K_thread_context *me)
{
	/* Convert to V2/V3 format */
	H2K_v2v3_entry_t tmp;
	tmp.pgsize = (Q6_R_ct0_R(list_entry.low)>>1);
	tmp.ppn = list_entry.ppd>>1;
	tmp.part = 0;
	tmp.ccc = list_entry.cccc;
	tmp.xwr = list_entry.xwr;
}

#else

u64_t H2K_mem_convert_to_native(H2K_linear_list_t list_entry, H2K_thread_context *me)
{
	/* Convert to V4 format */
	list_entry.high |= me->ssr_asid << 20;
	return list_entry.raw;
}

#endif

u64_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me)
{
	H2K_linear_list_t *list;
	list = (H2K_linear_list_t *)me->gptb;
	while (list->raw) {
		/* Insert Code Here */
		list++;
	}
	return H2K_mem_convert_to_native(list,me);
}

