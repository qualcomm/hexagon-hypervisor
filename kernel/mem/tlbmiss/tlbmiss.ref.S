/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>

FUNC_START H2K_handle_tlbmissx .text.tlbmiss .p2align 2
1:
	jump 1b
FUNC_END H2K_handle_tlbmissx

FUNC_START H2K_handle_tlbmissrw .text.tlbmiss .p2align 2
	jump 1b
FUNC_END H2K_handle_tlbmissrw

