/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CPUINT_H
#define H2K_CPUINT_H 1

#include <c_std.h>
#include <vm.h>
#include <context.h>
#include <vmint.h>

s32_t H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_cpuint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_cpuint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_cpuint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me) IN_SECTION(".text.vm.int");
s32_t H2K_vm_cpuint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me) IN_SECTION(".text.vm.int");
u32_t H2K_vm_cpuint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno) IN_SECTION(".text.vm.int");

extern const H2K_vm_int_ops_t H2K_vm_cpuint_ops;

#endif
