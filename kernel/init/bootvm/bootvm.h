/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <boot.h>
#include <asm_std.h>

FUNC_START __bootvm_entry .bootvm .p2align BOOT_TLB_ADDRBITS

//.incbin bootvm_image // here

