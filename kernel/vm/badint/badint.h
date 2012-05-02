/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_BADINT_H
#define H2K_BADINT_H 1

#include <c_std.h>
#include <vm.h>
#include <context.h>
#include <vmint.h>

s32_t H2K_vm_badint_func(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int");

extern const H2K_vm_int_ops_t H2K_vm_badint_ops;

#endif
