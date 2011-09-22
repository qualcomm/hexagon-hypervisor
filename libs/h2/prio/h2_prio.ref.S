/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_get_prio
	.global h2_set_prio
	.global h2_mask_prios_above
h2_get_prio:
	allocframe(#0)
	trap0(#H2_TRAP_PRIO_GET)
	deallocframe
	jumpr r31

h2_set_prio:
	allocframe(#0)
	trap0(#H2_TRAP_PRIO_SET)
	deallocframe
	jumpr r31

