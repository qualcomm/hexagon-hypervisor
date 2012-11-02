/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>

FUNC_START __boot .boot .p2align 12
jump start

.space 0x40

FUNC_END __boot
