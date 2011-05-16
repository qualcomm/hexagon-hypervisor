/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CREATE_H
#define H2K_CREATE_H 1

#include <context.h>
#include <vm.h>

s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me) IN_SECTION(".text.misc.create");

#endif
