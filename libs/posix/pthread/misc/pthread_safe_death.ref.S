/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * misc: helper to die safely while freeing our own TLS and stack
 */

#include <h2_common_asm_std.h>
#include <h2_trap_constants.h>

	.text

/*
 * extern void pthread_safe_death(pthread_plainmutex_t *unlock,pthread_t id) __attribute__((noreturn));
 */

FUNC_START pthread_safe_death .text.pthread.death .p2align 5
	{
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:t .Ldie
		r17:16 = r1:0
	}
1:
	call pthread_plainmutex_unlock_id_canfail_np
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) r1:0 = r17:16
		if (!p0.new) jump:nt 1b
	}
.Ldie:
	trap0(#H2_TRAP_THREAD_STOP)
FUNC_END pthread_safe_death

