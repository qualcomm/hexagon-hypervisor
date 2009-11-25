/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_register_fastint.S
 * This file contains the implementation for registering a fast interrupt
 */

#include "blast_trap_constants.h"

	.global blast_register_fastint
blast_register_fastint:
	allocframe(#0)
	trap0(#BLAST_TRAP_REGISTER_FASTINT)
	deallocframe
	jumpr r31

