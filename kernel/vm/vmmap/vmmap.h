/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMMAP_H
#define H2K_VMMAP_H 1

#include <c_std.h>
#include <context.h>

void H2K_vmtrap_clrmap(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_newmap(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");

#endif

