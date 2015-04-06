/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_tlb_trap
h2_tlb_trap:
	trap0(#H2_TRAP_TLBOP)
	jumpr r31

