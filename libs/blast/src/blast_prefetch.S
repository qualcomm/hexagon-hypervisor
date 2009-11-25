/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast_trap_constants.h>

	.text
	.global blast_set_prefetch
blast_set_prefetch:
	allocframe(#0)
	trap0(#BLAST_TRAP_SET_PREFETCH)
	deallocframe
	jumpr r31

