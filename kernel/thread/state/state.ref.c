/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <state.h>
#include <asm_offsets.h>
#include <vmblock.h>

IN_SECTION(".text.core.state") u64_t H2K_thread_state(H2K_id_t id, u32_t offset, H2K_thread_context *me) {

	u64_t val;
	u32_t i;
	u8_t *p = (u8_t *)&val;

	// check for bad VM, bad offset, bad cpuidx
	if ((id.vmidx != me->id.vmidx)
			||(offset > CONTEXT_SIZE - 8)
			|| (id.cpuidx >= me->vmblock->max_cpus)) {

		return -1ULL;
	}

	// copy by bytes in case offset unaligned
	for (i = 0; i < 8; i++) {
		*p = *(((u8_t *)&me->vmblock->contexts[id.cpuidx]) + offset + i);
		p++;
	}

	return val;
}

