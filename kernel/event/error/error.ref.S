/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START BLASTK_handle_error .text.error .p2align 2
1:
	jump 1b
FUNC_END BLASTK_handle_error

FUNC_START BLASTK_handle_nmi .text.error .p2align 2
	jump 1b
FUNC_END BLASTK_handle_nmi

FUNC_START BLASTK_handle_rsvd .text.error .p2align 2
	jump 1b
FUNC_END BLASTK_handle_nmi

FUNC_START BLASTK_handle_trap1 .text.error .p2align 2
	jump 1b
FUNC_END BLASTK_handle_trap1

