/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SHINT_H
#define H2K_SHINT_H 1

#include <c_std.h>
#include <vm.h>
#include <context.h>
#include <vmint.h>

s32_t H2K_vm_shint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_localmask(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_localunmask(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_setaffinity(H2K_vmblock_t *vmblock, u32_t idx, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me) IN_SECTION(".text.vm.int");
s32_t H2K_vm_shint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me) IN_SECTION(".text.vm.int");
u32_t H2K_vm_shint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");

extern const H2K_vm_int_ops_t H2K_vm_shint_ops;

#endif
