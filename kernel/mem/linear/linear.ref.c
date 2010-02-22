/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <linear.h>
#include <q6protos.h>
#include <tlbfmt.h>

u64_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me)
{
	H2K_linear_list_t *list;
	list = (H2K_linear_list_t *)me->gptb;
	while (list->raw) {
		/* Insert Code Here */
		list++;
	}
	return H2K_mem_tlbfmt_from_linear(list,me->asid);
}

