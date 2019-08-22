/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VM_H
#define H2K_VM_H 1

#include<asid_types.h>
#include <vmblock.h>
#include <translate.h>

void H2K_vmblock_clear(H2K_vmblock_t *vmblock) IN_SECTION(".text.data.vm");

#endif

