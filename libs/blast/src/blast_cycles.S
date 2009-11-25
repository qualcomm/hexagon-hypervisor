/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast_trap_constants.h>

	.text
	.global blast_get_pcycles
blast_get_pcycles:
	allocframe(#0)
	trap0(#BLAST_TRAP_CPUTIME)
1:
	deallocframe
	jumpr r31

	.global blast_get_core_pcycles
blast_get_core_pcycles:
	allocframe(#0)
	trap0(#BLAST_TRAP_GET_PCYCLES)
	jump 1b

