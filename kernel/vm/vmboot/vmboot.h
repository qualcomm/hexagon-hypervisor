/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMBOOT_H
#define H2K_VMBOOT_H 1

#include <vm.h>
#include <context.h>

s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");

u32_t H2K_vmstatus(u32_t vm, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");

u32_t H2K_vmfree(u32_t vm, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");
