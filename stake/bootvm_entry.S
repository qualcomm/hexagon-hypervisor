/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>

// 
FUNC_START __bootvm_entry_point .entry .p2align 12
jump hexagon_pre_main

FUNC_END __bootvm_entry_point
