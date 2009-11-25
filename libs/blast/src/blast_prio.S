/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "blast_trap_constants.h"

	.text
	.global blast_prio_get
	.global blast_prio_set
	.global blast_mask_prios_above
blast_prio_get:
	allocframe(#0)
	trap0(#BLAST_TRAP_PRIO_GET)
	deallocframe
	jumpr r31

blast_prio_set:
	allocframe(#0)
	trap0(#BLAST_TRAP_PRIO_SET)
	deallocframe
	jumpr r31

blast_mask_prios_above:
	allocframe(#0)
	trap0(#BLAST_TRAP_MASK_PRIOS_ABOVE)
	deallocframe
	jumpr r31

