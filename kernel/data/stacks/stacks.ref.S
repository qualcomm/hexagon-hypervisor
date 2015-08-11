/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

// FIXME: get from allocator?

DATA_START H2K_stack_area .data.core.stacks .p2align 5
	.space KERNEL_STACK_SIZE
	.global H2K_stacks
H2K_stacks:
	.space (KERNEL_STACK_SIZE * (MAX_HTHREADS - 1))

DATA_END H2K_stack_area 

