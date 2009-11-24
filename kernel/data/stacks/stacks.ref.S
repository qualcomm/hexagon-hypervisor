/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

DATA_START BLASTK_stack_area .data.stacks .p2align 5
	.space KERNEL_STACK_SIZE
	.global BLASTK_stacks
BLASTK_stacks:
	.space (KERNEL_STACK_SIZE * (MAX_HTHREADS - 1))

DATA_END BLASTK_stack_area 

