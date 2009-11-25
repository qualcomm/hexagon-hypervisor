/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_intwait.S
 * 
 * This file contains the implementation for 
 * blast_intwait functionality 
 */

#include "blast_trap_constants.h"

	.text
	.global blast_intwait

	/* blast_intwait
	 * Provide a mask.  Basically, we call trap and return.
	 */
blast_intwait:
	allocframe(#0)
	trap0(#BLAST_TRAP_INTWAIT)
	deallocframe
	jumpr r31

